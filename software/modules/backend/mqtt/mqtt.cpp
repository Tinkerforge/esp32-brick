#include "mqtt.h"

#include <ESPmDNS.h>
#include <SPIFFS.h>

#include "Arduino.h"

#include "ArduinoJson.h"
#include "AsyncJson.h"

#include "modules/task_scheduler/task_scheduler.h"

#include "tools.h"
#include "api.h"

extern TaskScheduler task_scheduler;
extern AsyncWebServer server;
extern AsyncEventSource events;
extern char uid[7];
extern API api;

Mqtt::Mqtt() {

}

void Mqtt::apply_config() {
    this->mqttClient.setClientId(mqtt_config_in_use.get("client_name")->asString().c_str());
    this->mqttClient.setCredentials(mqtt_config_in_use.get("broker_username")->asString().c_str(),
                                    mqtt_config_in_use.get("broker_password")->asString().c_str());

    IPAddress ip_addr;
    if(ip_addr.fromString(mqtt_config_in_use.get("broker_host")->asString().c_str())) {
        this->mqttClient.setServer(ip_addr, mqtt_config_in_use.get("broker_port")->asUint());
    } else {
        this->mqttClient.setServer(mqtt_config_in_use.get("broker_host")->asString().c_str(), mqtt_config_in_use.get("broker_port")->asUint());
    }

    if(this->mqtt_config_in_use.get("enable_mqtt")->asBool())
        this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::NOT_CONNECTED);
}

void Mqtt::connect() {
    if(!initialized)
        return;

    if(!this->mqtt_config_in_use.get("enable_mqtt")->asBool())
        return;

    this->mqttClient.connect();
}

void Mqtt::publish(String topic_suffix, String payload)
{
    String prefix = mqtt_config_in_use.get("global_topic_prefix")->asString();
    String topic = prefix + "/" + topic_suffix;
    this->mqttClient.publish(topic.c_str(), payload.c_str(), payload.length(), 1, true);
}

void Mqtt::subscribe(String topic_suffix, uint32_t max_payload_length, std::function<void(String)> callback)
{
    String prefix = mqtt_config_in_use.get("global_topic_prefix")->asString();
    String topic = prefix + "/" + topic_suffix;
    this->commands.push_back({topic, max_payload_length, callback});
    this->mqttClient.subscribe(topic.c_str(), 2);
}

void Mqtt::onMqttError(uint8_t e,uint32_t info){
  switch(e){
    case TCP_DISCONNECTED:
        // usually because your structure is wrong and you called a function before onMqttConnect
        // Serial.printf("ERROR: NOT CONNECTED info=%d\n",info);
        // Ignore not connected errors, this allows us to not check this every time we publish
        break;
    case MQTT_SERVER_UNAVAILABLE:
        // server has gone away - network problem? server crash?
        Serial.printf("ERROR: MQTT_SERVER_UNAVAILABLE info=%d\n",info);
        this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::ERROR);
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    case UNRECOVERABLE_CONNECT_FAIL:
        // there is something wrong with your connection parameters? IP:port incorrect? user credentials typo'd?
        Serial.printf("ERROR: UNRECOVERABLE_CONNECT_FAIL info=%d\n",info);
        this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::ERROR);
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    case TLS_BAD_FINGERPRINT:
        Serial.printf("ERROR: TLS_BAD_FINGERPRINT info=%d\n",info);
        this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::ERROR);
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    case SUBSCRIBE_FAIL:
        // you tried to subscribe to an invalid topic
        Serial.printf("ERROR: SUBSCRIBE_FAIL info=%d\n",info);
        this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::ERROR);
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    case INBOUND_QOS_ACK_FAIL:
        Serial.printf("ERROR: OUTBOUND_QOS_ACK_FAIL id=%d\n",info);
        this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::ERROR);
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    case OUTBOUND_QOS_ACK_FAIL:
        Serial.printf("ERROR: OUTBOUND_QOS_ACK_FAIL id=%d\n",info);
        this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::ERROR);
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    case INBOUND_PUB_TOO_BIG:
        // someone sent you a p[acket that this MCU does not have enough FLASH to handle
        Serial.printf("ERROR: INBOUND_PUB_TOO_BIG size=%d Max=%d\n",e,mqttClient.getMaxPayloadSize());
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    case OUTBOUND_PUB_TOO_BIG:
        // you tried to send a packet that this MCU does not have enough FLASH to handle
        Serial.printf("ERROR: OUTBOUND_PUB_TOO_BIG size=%d Max=%d\n",e,mqttClient.getMaxPayloadSize());
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    case BOGUS_PACKET: //  Your server sent a control packet type unknown to MQTT 3.1.1
    //  99.99% unlikely to ever happen, but this message is better than a crash, non?
        Serial.printf("ERROR: BOGUS_PACKET TYPE=%02x\n",e,info);
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    case X_INVALID_LENGTH: //  An x function rcvd a msg with an unexpected length: probale data corruption or malicious msg
    //  99.99% unlikely to ever happen, but this message is better than a crash, non?
        Serial.printf("ERROR: X_INVALID_LENGTH TYPE=%02x\n",e,info);
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    default:
        Serial.printf("UNKNOWN ERROR: %u extra info %d",e,info);
        this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::ERROR);
        this->mqtt_state.get("last_error")->updateInt(255);
        break;
  }
}

void Mqtt::onMqttConnect(bool sessionPresent) {
    Serial.printf("Connected to MQTT session=%d max payload size=%d\r\n",sessionPresent,mqttClient.getMaxPayloadSize());
    this->was_connected = true;
    this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::CONNECTED);

    api.onMqttConnect();
}

void Mqtt::onMqttMessage(const char* topic, const uint8_t* payload, size_t len,uint8_t qos,bool retain,bool dup) {
    if(dup)
        return;

    for(auto &c : commands) {
        Serial.println(String("checking ") + c.topic);
        if(c.topic != topic)
            continue;
        Serial.println("found");

        if(len > c.max_len)
            break;
        Serial.println("len ok");

        String s;
        mqttClient.xPayload(payload, len, s);
        c.callback(s);
    }
}

void Mqtt::onMqttDisconnect(int8_t reason) {
    this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::NOT_CONNECTED);
    if(this->was_connected) {
        Serial.printf("Disconnected from MQTT reason=%d\n",reason);
        this->was_connected = false;
    }
    task_scheduler.scheduleOnce("reconnect MQTT", [this](){this->connect();}, 5000);
}

void Mqtt::setup()
{
    // This can't be done in the constructor, as the uid is then still unknown.
    mqtt_config = Config::Object({
        {"enable_mqtt", Config::Bool(false)},
        {"broker_host", Config::Str("", 128)},
        {"broker_port", Config::Uint16(1883)},
        {"broker_username", Config::Str("", 64)},
        {"broker_password", Config::Str("", 64)},
        {"global_topic_prefix", Config::Str(String(__HOST_PREFIX__) + String("/") + String(uid), 64)},
        {"client_name", Config::Str(String(__HOST_PREFIX__) + String("-") + String(uid), 64)}
    });

    mqtt_state = Config::Object({
        {"connection_state", Config::Int(0)},
        {"last_error", Config::Int(-1)}
    });

    mqttClient.onConnect([this](bool session){this->onMqttConnect(session);});
    mqttClient.onDisconnect([this](int8_t reason){this->onMqttDisconnect(reason);});
    mqttClient.onMessage([this](const char* topic, const uint8_t* payload, size_t len, uint8_t qos, bool retain, bool dup){this->onMqttMessage(topic, payload, len, qos, retain, dup);});
    //mqttClient.setWill("DIED",2,false,"probably still some bugs");
    mqttClient.onError([this](uint8_t error, int info){this->onMqttError(error, info);});
    mqttClient.setCleanSession(true);
    mqttClient.setKeepAlive(15);

    //TODO read .tmp if real file does not exist
    if(SPIFFS.exists("/mqtt_config.json")) {
        File file = SPIFFS.open("/mqtt_config.json");
        String error = mqtt_config.update_from_file(file);
        file.close();
        if(error != "")
            Serial.println(error);
    }

    mqtt_config_in_use = mqtt_config;
    apply_config();

    initialized = true;
    connect();
}

void Mqtt::register_urls()
{
    api.addPersistentConfig("mqtt_config", &mqtt_config, {"broker_password"}, 1000);
    api.addState("mqtt_state", &mqtt_state, {}, 1000);
}

void Mqtt::onEventConnect(AsyncEventSourceClient *client)
{

}

void Mqtt::loop()
{

}
