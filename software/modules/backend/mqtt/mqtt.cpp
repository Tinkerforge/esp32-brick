#include "mqtt.h"

#include <ESPmDNS.h>
#include <SPIFFS.h>

#include "Arduino.h"

#include "ArduinoJson.h"
#include "AsyncJson.h"

#include "../wifi/wifi.h"

#include "task_scheduler.h"
#include "tools.h"
#include "api.h"
#include "event_log.h"

extern EventLog logger;

extern TaskScheduler task_scheduler;
extern char uid[7];
extern API api;

extern Wifi wifi;

Mqtt::Mqtt() {
    api.registerBackend(this);
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

    auto state = (MqttConnectionState)this->mqtt_state.get("connection_state")->asInt();
    if(state != MqttConnectionState::NOT_CONNECTED && state != MqttConnectionState::ERROR) {
        return;
    }

    if (wifi.state == WifiState::CONNECTING)
        return;

    this->mqttClient.connect();
}

void Mqtt::publish(String topic_suffix, String payload)
{
    if(this->mqtt_state.get("connection_state")->asInt() != (int)MqttConnectionState::CONNECTED) {
        return;
    }
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

void Mqtt::addCommand(CommandRegistration reg)
{
    subscribe(reg.path, reg.config->json_size(), [reg](String payload){
        String error = reg.config->update_from_string(payload);
        if(error == "")
            task_scheduler.scheduleOnce((String("notify command update for ") + reg.path).c_str(), [reg](){reg.callback();}, 0);
        else
            logger.printfln("%s", error.c_str());
    });
}

void Mqtt::addState(StateRegistration reg)
{

}

void Mqtt::pushStateUpdate(String payload, String path)
{
    publish(path, payload);
}

void Mqtt::onMqttError(uint8_t e,uint32_t info){
  switch(e){
    case TCP_DISCONNECTED:
        // usually because your structure is wrong and you called a function before onMqttConnect
        // Serial.printf("ERROR: NOT CONNECTED info=%d\n",info);
        // Ignore not connected errors, this allows us to not check this every time we publish
        /*this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::ERROR);
        this->mqtt_state.get("last_error")->updateInt(e);*/
        break;
    case MQTT_SERVER_UNAVAILABLE:
        // server has gone away - network problem? server crash?
        logger.printfln("ERROR: MQTT_SERVER_UNAVAILABLE info=%d",info);
        this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::ERROR);
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    case UNRECOVERABLE_CONNECT_FAIL:
        // there is something wrong with your connection parameters? IP:port incorrect? user credentials typo'd?
        logger.printfln("ERROR: UNRECOVERABLE_CONNECT_FAIL info=%d",info);
        this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::ERROR);
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    case TLS_BAD_FINGERPRINT:
        logger.printfln("ERROR: TLS_BAD_FINGERPRINT info=%d",info);
        this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::ERROR);
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    case SUBSCRIBE_FAIL:
        // you tried to subscribe to an invalid topic
        logger.printfln("ERROR: SUBSCRIBE_FAIL info=%d",info);
        this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::ERROR);
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    case INBOUND_QOS_ACK_FAIL:
        logger.printfln("ERROR: OUTBOUND_QOS_ACK_FAIL id=%d",info);
        this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::ERROR);
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    case OUTBOUND_QOS_ACK_FAIL:
        logger.printfln("ERROR: OUTBOUND_QOS_ACK_FAIL id=%d",info);
        this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::ERROR);
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    case INBOUND_PUB_TOO_BIG:
        // someone sent you a p[acket that this MCU does not have enough FLASH to handle
        logger.printfln("ERROR: INBOUND_PUB_TOO_BIG size=%d Max=%d",e,mqttClient.getMaxPayloadSize());
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    case OUTBOUND_PUB_TOO_BIG:
        // you tried to send a packet that this MCU does not have enough FLASH to handle
        logger.printfln("ERROR: OUTBOUND_PUB_TOO_BIG size=%d Max=%d",e,mqttClient.getMaxPayloadSize());
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    case BOGUS_PACKET: //  Your server sent a control packet type unknown to MQTT 3.1.1
    //  99.99% unlikely to ever happen, but this message is better than a crash, non?
        logger.printfln("ERROR: BOGUS_PACKET TYPE=%02x",e);
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    case X_INVALID_LENGTH: //  An x function rcvd a msg with an unexpected length: probale data corruption or malicious msg
    //  99.99% unlikely to ever happen, but this message is better than a crash, non?
        logger.printfln("ERROR: X_INVALID_LENGTH TYPE=%02x",e);
        this->mqtt_state.get("last_error")->updateInt(e);
        break;
    default:
        logger.printfln("UNKNOWN ERROR: %u extra info %d",e,info);
        this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::ERROR);
        this->mqtt_state.get("last_error")->updateInt(255);
        break;
  }
}

void Mqtt::onMqttConnect(bool sessionPresent) {
    logger.printfln("Connected to MQTT session=%d max payload size=%d",sessionPresent,mqttClient.getMaxPayloadSize());
    this->was_connected = true;
    this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::CONNECTED);

    // Do the publishing in the "main thread". Otherwise this would be a race condition with the publishing in addState.
    task_scheduler.scheduleOnce("onMqttConnect", [this](){
        for(auto &reg : api.commands) {
            subscribe(reg.path, reg.config->json_size(), [reg](String payload){
                String error = reg.config->update_from_string(payload);
                if(error == "")
                    task_scheduler.scheduleOnce((String("notify command update for ") + reg.path).c_str(), [reg](){reg.callback();}, 0);
                else
                    logger.printfln("%s", error.c_str());
            });
        }
        for(auto &reg : api.states) {
            publish(reg.path, reg.config->to_string_except(reg.keys_to_censor));
        }
    }, 0);
}

void Mqtt::onMqttMessage(const char* topic, const uint8_t* payload, size_t len,uint8_t qos,bool retain,bool dup) {
    if(dup)
        return;

    for(auto &c : commands) {
        if(c.topic != topic)
            continue;

        if(len > c.max_len)
            break;

        String s;
        mqttClient.xPayload(payload, len, s);
        c.callback(s);
    }
}

String mqttError(int8_t reason) {
    switch (reason) {
        //Pangolin errors
        case TCP_DISCONNECTED:
            return "TCP_DISCONNECTED";
        case MQTT_SERVER_UNAVAILABLE:
            return "MQTT_SERVER_UNAVAILABLE";
        case UNRECOVERABLE_CONNECT_FAIL:
            return "UNRECOVERABLE_CONNECT_FAIL";
        case TLS_BAD_FINGERPRINT:
            return "TLS_BAD_FINGERPRINT";
        case SUBSCRIBE_FAIL:
            return "SUBSCRIBE_FAIL";
        case INBOUND_QOS_ACK_FAIL:
            return "INBOUND_QOS_ACK_FAIL";
        case OUTBOUND_QOS_ACK_FAIL:
            return "OUTBOUND_QOS_ACK_FAIL";
        case INBOUND_PUB_TOO_BIG:
            return "INBOUND_PUB_TOO_BIG";
        case OUTBOUND_PUB_TOO_BIG:
            return "OUTBOUND_PUB_TOO_BIG";
        case BOGUS_PACKET:
            return "BOGUS_PACKET";
        case X_INVALID_LENGTH:
            return "X_INVALID_LENGTH";

        // LWIP errors
        case -1:
            return "Out of memory error (ERR_MEM)";
        case -2:
            return "Buffer error (ERR_BUF)";
        case -3:
            return "Timeout (ERR_TIMEOUT)";
        case -4:
            return "Routing problem (ERR_RTE)";
        case -5:
            return "Operation in progress (ERR_INPROGRESS)";
        case -6:
            return "Illegal value (ERR_VAL)";
        case -7:
            return "Operation would block (ERR_WOULDBLOCK)";
        case -8:
            return "Address in use (ERR_USE)";
        case -9:
            return "Already connecting (ERR_ALREADY)";
        case -10:
            return "Conn already established (ERR_ISCONN)";
        case -11:
            return "Not connected (ERR_CONN)";
        case -12:
            return "Low-level netif error (ERR_IF)";
        case -13:
            return "Connection aborted (ERR_ABRT)";
        case -14:
            return "Connection reset (ERR_RST)";
        case -15:
            return "Connection closed (ERR_CLSD)";
        case -16:
            return "Illegal argument (ERR_ARG)";

        // Async TCP errors
        case -55:
            return "Can't resolve hostname";
    }
}

void Mqtt::onMqttDisconnect(int8_t reason) {
    this->mqtt_state.get("connection_state")->updateInt((int)MqttConnectionState::NOT_CONNECTED);
    if(this->was_connected) {
        logger.printfln("Disconnected from MQTT: %s (%d)", mqttError(reason).c_str(), reason);
        this->was_connected = false;
    }
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
            logger.printfln(error.c_str());
    }

    mqtt_config_in_use = mqtt_config;
    apply_config();

    initialized = true;
    task_scheduler.scheduleWithFixedDelay("reconnect MQTT", [this](){this->connect();}, 5000, 5000);
}

void Mqtt::register_urls()
{
    api.addPersistentConfig("mqtt/config", &mqtt_config, {"broker_password"}, 1000);
    api.addState("mqtt/state", &mqtt_state, {}, 1000);
}


void Mqtt::loop()
{

}
