#include "wifi.h"

#include <ESPmDNS.h>
#include <SPIFFS.h>

#include "ArduinoJson.h"
#include "AsyncJson.h"

#include "modules/task_scheduler/task_scheduler.h"

#include "tools.h"

#define RECONNECT_TIMEOUT_MS 30000

extern TaskScheduler task_scheduler;
extern AsyncWebServer server;
extern AsyncEventSource events;
extern char uid[7];
extern char passphrase[20];

String update_config(Config &cfg, String config_name, JsonVariant &json) {
    String error = cfg.update_from_json(json);

    String tmp_path = String("/") + config_name + ".json.tmp";
    String path = String("/") + config_name + ".json";

    if (error == "") {
        if (SPIFFS.exists(tmp_path))
            SPIFFS.remove(tmp_path);

        File file = SPIFFS.open(tmp_path, "w");
        cfg.save_to_file(file);
        file.close();

        if (SPIFFS.exists(path))
            SPIFFS.remove(path);

        SPIFFS.rename(tmp_path, path);
        Serial.print(path);
        Serial.println(" updated");
    } else {
        Serial.print("Failed to update ");
        Serial.print(path);
        Serial.print(":\n    ");
        Serial.println(error);
    }
    return error;
}

Wifi::Wifi() {
    wifi_config = Config::Object({
        {"enable_soft_ap", Config::Bool(true)},
        {"enable_sta", Config::Bool(false)},
        {"ap_fallback_only", Config::Bool(false)},
    });


    wifi_soft_ap_config = Config::Object({
        {"ssid",  Config::Str("esp-brick", 32)},
        {"hide_ssid", Config::Bool(false)},
        {"passphrase", Config::Str("0123456789", 64, [](const Config::ConfString &s) {
                return (s.value.length() >= 8 && s.value.length() <= 63) || //FIXME: check if there are only ASCII characters here.
                    (s.value.length() == 64) ? String("") : String("passphrase must be of length 8 to 63, or 64 if PSK."); //FIXME: check if there are only hex digits here.
            })
        },
        {"hostname", Config::Str("esp-brick", 32)},
        {"channel", Config::Uint(1, 1, 13)},
        {"ip", Config::Array({
                Config::Uint8(10),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(1),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
        {"gateway", Config::Array({
                Config::Uint8(10),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(1),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
        {"subnet", Config::Array({
                Config::Uint8(255),
                Config::Uint8(255),
                Config::Uint8(255),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
    });
    wifi_sta_config = Config::Object({
        {"ssid", Config::Str("", 32)},
        {"bssid", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0)
                },
                Config::Uint8(0),
                6,
                6,
                Config::type_id<Config::ConfUint>()
            )
        },
        {"bssid_lock", Config::Bool(false)},
        {"passphrase", Config::Str("", 64, [](const Config::ConfString &s) {
                return s.value.length() == 0 ||
                    (s.value.length() >= 8 && s.value.length() <= 63) || //FIXME: check if there are only ASCII characters here.
                    (s.value.length() == 64) ? String("") : String("passphrase must be of length zero, or 8 to 63, or 64 if PSK."); //FIXME: check if there are only hex digits here.
            })
        },
        {"hostname", Config::Str("wallbox", 32)},
        {"ip", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
        {"gateway", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
        {"subnet", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
        {"dns", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
        {"dns2", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
    });

    wifi_state = Config::Object({
        {"connection_state", Config::Int(0)},
        {"ap_state", Config::Int(0)}
    });
}

void Wifi::apply_soft_ap_config_and_start() {
    uint8_t ip[4];
    uint8_t gateway[4];
    uint8_t subnet[4];

    wifi_soft_ap_config_in_use.get("ip")->fillArray<uint8_t, Config::ConfUint>(ip, 4);
    wifi_soft_ap_config_in_use.get("gateway")->fillArray<uint8_t, Config::ConfUint>(gateway, 4);
    wifi_soft_ap_config_in_use.get("subnet")->fillArray<uint8_t, Config::ConfUint>(subnet, 4);

    WiFi.softAPConfig(ip, gateway, subnet);

    printf("Soft AP started.\n");
    printf("    SSID: %s\n", wifi_soft_ap_config_in_use.get("ssid")->asString().c_str());
    printf("    passphrase: %s\n", wifi_soft_ap_config_in_use.get("passphrase")->asString().c_str());
    printf("    hostname: %s\n", wifi_soft_ap_config_in_use.get("hostname")->asString().c_str());

    WiFi.softAPsetHostname(wifi_soft_ap_config_in_use.get("hostname")->asString().c_str());

    WiFi.softAP(wifi_soft_ap_config_in_use.get("ssid")->asString().c_str(),
                wifi_soft_ap_config_in_use.get("passphrase")->asString().c_str(),
                wifi_soft_ap_config_in_use.get("channel")->asUint(),
                wifi_soft_ap_config_in_use.get("hide_ssid")->asBool());

    soft_ap_running = true;
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("    IP: ");
    Serial.println(myIP);
    /*Serial.print("Subnet CIDR: ");
    Serial.println(WiFi.softAPSubnetCIDR());
    Serial.print("Broadcast IP: ");
    Serial.println(WiFi.softAPBroadcastIP());*/
}

void Wifi::apply_sta_config_and_connect() {
    String ssid = wifi_sta_config_in_use.get("ssid")->asString();

    uint8_t bssid[6];
    wifi_sta_config_in_use.get("bssid")->fillArray<uint8_t, Config::ConfUint>(bssid, 6 * sizeof(bssid));

    String passphrase = wifi_sta_config_in_use.get("passphrase")->asString();
    bool bssid_lock = wifi_sta_config_in_use.get("bssid_lock")->asBool();

    uint8_t ip[4], subnet[4], gateway[4], dns[4], dns2[4];
    wifi_sta_config_in_use.get("ip")->fillUint8Array(ip, 4);
    wifi_sta_config_in_use.get("subnet")->fillUint8Array(subnet, 4);
    wifi_sta_config_in_use.get("gateway")->fillUint8Array(gateway, 4);
    wifi_sta_config_in_use.get("dns")->fillUint8Array(dns, 4);
    wifi_sta_config_in_use.get("dns2")->fillUint8Array(dns2, 4);

    WiFi.disconnect(false, false);

    WiFi.begin(ssid.c_str(), passphrase.c_str(), 0, bssid_lock ? bssid : nullptr, false);

    if(ip != 0) {
        WiFi.config(ip, gateway, subnet, dns, dns2);
    } else {
        WiFi.config((uint32_t)0, (uint32_t)0, (uint32_t)0);
    }

    WiFi.setHostname(wifi_sta_config_in_use.get("hostname")->asString().c_str());

    connect_to_wifi();
}

void Wifi::setup()
{
    String default_hostname = String(__HOST_PREFIX__) + String(uid);
    String default_passphrase = String(passphrase);
    WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {this->state = WifiState::NOT_CONNECTED; Serial.println("Wifi disconnected");}, SYSTEM_EVENT_STA_DISCONNECTED);
    WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {this->state = WifiState::CONNECTED; Serial.println("Wifi connected");}, SYSTEM_EVENT_STA_CONNECTED);

    //TODO read .tmp if real file does not exist
    if(SPIFFS.exists("/wifi_config.json")) {
        File file = SPIFFS.open("/wifi_config.json");
        String error = wifi_config.update_from_file(file);
        file.close();
        if(error != "")
            Serial.println(error);
    }

    if(SPIFFS.exists("/wifi_sta_config.json")) {
        File file = SPIFFS.open("/wifi_sta_config.json");
        String error = wifi_sta_config.update_from_file(file);
        file.close();
        if(error != "")
            Serial.println(error);
    } /*else {
        wifi_sta_config.get("hostname")->updateString(default_hostname);
    }*/

    if(SPIFFS.exists("/wifi_soft_ap_config.json")) {
        File file = SPIFFS.open("/wifi_soft_ap_config.json");
        String error = wifi_soft_ap_config.update_from_file(file);
        file.close();
        if(error != "")
            Serial.println(error);
    } else {
        wifi_soft_ap_config.get("hostname")->updateString(default_hostname);
        wifi_soft_ap_config.get("ssid")->updateString(default_hostname);
        wifi_soft_ap_config.get("passphrase")->updateString(default_passphrase);
        File file = SPIFFS.open("/wifi_soft_ap_config.json", "w");
        wifi_soft_ap_config.save_to_file(file);
        file.close();
    }

    wifi_config_in_use = wifi_config;
    wifi_soft_ap_config_in_use = wifi_soft_ap_config;
    wifi_sta_config_in_use = wifi_sta_config;

    bool enable_soft_ap = wifi_config_in_use.get("enable_soft_ap")->asBool();
    bool enable_sta = wifi_config_in_use.get("enable_sta")->asBool();
    bool ap_fallback_only = wifi_config_in_use.get("ap_fallback_only")->asBool();

    if (enable_sta && enable_soft_ap) {
        WiFi.mode(WIFI_AP_STA);
    } else if (enable_soft_ap) {
        WiFi.mode(WIFI_AP);
    } else if (enable_sta) {
        WiFi.mode(WIFI_STA);
    } else {
        WiFi.mode(WIFI_OFF);
    }

    if (enable_soft_ap && !ap_fallback_only) {
        apply_soft_ap_config_and_start();
    }

    if (enable_sta) {
        apply_sta_config_and_connect();
    }

    /*use mdns for host name resolution*/
    if (!MDNS.begin(wifi_soft_ap_config_in_use.get("hostname")->asString().c_str())) {
        Serial.println("Error setting up mDNS responder!");
    } else {
        Serial.println("mDNS responder started");
    }
}

void Wifi::register_urls()
{
    server.on("/wifi_state", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "application/json; charset=utf-8", wifi_state_str());
        return;
    });
    server.on("/scan_wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
        WiFi.scanDelete();

        // WIFI_SCAN_FAILED also means the scan is done.
        if(WiFi.scanComplete() == WIFI_SCAN_FAILED){
            WiFi.scanNetworks(true, true);
        }

        request->send(204, "", "");
    });
    server.on("/get_wifis", HTTP_GET, [](AsyncWebServerRequest *request) {
        int network_count = WiFi.scanComplete();

        Serial.println("scan done");

        //result line: {"ssid": "%s", "bssid": "%s", "rssi": %d, "channel": %d, "encryption": %d}
        //worst case length ~ 140

        if (network_count == 0) {
            request->send(200, "application/json; charset=utf-8", "{\"result\": []}");
        } else {
            String result;
            result.reserve(145 * network_count);
            Serial.print(network_count);
            Serial.println(" networks found");
            result += "{\"result\":[";

            for (int i = 0; i < network_count; ++i) {
                // Print SSID and RSSI for each network found
                result += "{\"ssid\": \"";
                result += WiFi.SSID(i);
                result += "\", \"bssid\": \"";
                result += WiFi.BSSIDstr(i);
                result += "\", \"rssi\": ";
                result += WiFi.RSSI(i);
                result += ", \"channel\": ";
                result += WiFi.channel(i);
                result += ", \"encryption\": ";
                result += WiFi.encryptionType(i);
                result += "}";
                if(i != network_count - 1)
                    result += ",";
            }
            result += "]}";
            request->send(200, "application/json; charset=utf-8", result);
        }
    });

    server.on("/wifi_sta_config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        auto *response = request->beginResponseStream("application/json; charset=utf-8");
        wifi_sta_config.write_to_stream_except(*response, {"passphrase"});
        request->send(response);
    });
    AsyncCallbackJsonWebHandler *wifi_sta_config_handler = new AsyncCallbackJsonWebHandler("/wifi_sta_config", [this](AsyncWebServerRequest *request, JsonVariant &json){
        String message = update_config(wifi_sta_config, "wifi_sta_config", json);

        if (message == "") {
            request->send(200, "text/html", "Settings saved");
        } else {
            request->send(400, "text/html", message);
        }
    });
    server.addHandler(wifi_sta_config_handler);

    server.on("/wifi_config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        auto *response = request->beginResponseStream("application/json; charset=utf-8");
        wifi_config.write_to_stream(*response);
        request->send(response);
    });
    AsyncCallbackJsonWebHandler *wifi_config_handler = new AsyncCallbackJsonWebHandler("/wifi_config", [this](AsyncWebServerRequest *request, JsonVariant &json){
        String message = update_config(wifi_config, "wifi_config", json);

        if (message == "") {
            request->send(200, "text/html", "Settings saved");
        } else {
            request->send(400, "text/html", message);
        }
    });
    server.addHandler(wifi_config_handler);

    server.on("/wifi_soft_ap_config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        auto *response = request->beginResponseStream("application/json; charset=utf-8");
        wifi_soft_ap_config.write_to_stream(*response);
        request->send(response);
    });
    AsyncCallbackJsonWebHandler *wifi_soft_ap_config_handler = new AsyncCallbackJsonWebHandler("/wifi_soft_ap_config", [this](AsyncWebServerRequest *request, JsonVariant &json){
        String message = update_config(wifi_soft_ap_config, "wifi_soft_ap_config", json);

        if (message == "") {
            request->send(200, "text/html", "Settings saved");
        } else {
            request->send(400, "text/html", message);
        }
    });
    server.addHandler(wifi_soft_ap_config_handler);
}

void Wifi::onEventConnect(AsyncEventSourceClient *client)
{
    String wifi_config_str = this->wifi_config_str();
    client->send(wifi_config_str.c_str(), "wifi_config", millis(), 1000);

    String wifi_state_str = this->wifi_state_str();
    client->send(wifi_state_str.c_str(), "wifi_state", millis(), 1000);

    String wifi_sta_config_str = this->wifi_sta_config_str();
    client->send(wifi_sta_config_str.c_str(), "wifi_sta_config", millis(), 1000);
}

void Wifi::loop()
{
    bool send_event = false;
    send_event |= wifi_state.get("connection_state")->updateInt(get_connection_state());
    send_event |= wifi_state.get("ap_state")->updateInt(get_ap_state());

    if(send_event && send_event_allowed(&events)) {
        String str = wifi_state_str();
        events.send(str.c_str(), "wifi_state", millis());
    }

    if (wifi_config_in_use.get("enable_sta")->asBool() &&
        wifi_config_in_use.get("enable_soft_ap")->asBool() &&
        wifi_config_in_use.get("ap_fallback_only")->asBool() &&
        WiFi.status() != WL_CONNECTED &&
        !soft_ap_running) {
            apply_soft_ap_config_and_start();
    }

    if (wifi_config_in_use.get("enable_sta")->asBool() &&
        wifi_config_in_use.get("ap_fallback_only")->asBool() &&
        WiFi.status() == WL_CONNECTED) {
        WiFi.softAPdisconnect(true);
        soft_ap_running = false;
    }
}

String Wifi::wifi_config_str()
{
    return wifi_config.to_string();
}

String Wifi::wifi_soft_ap_config_str()
{
    return wifi_soft_ap_config.to_string();
}

String Wifi::wifi_sta_config_str()
{
    return wifi_sta_config.to_string_except({"passphrase"});
}

String Wifi::wifi_state_str() {
    return wifi_state.to_string();
}

int Wifi::get_connection_state() {
    if (!wifi_config_in_use.get("enable_sta")->asBool())
        return -1;

    switch(WiFi.status()) {
        case WL_CONNECT_FAILED:
        case WL_CONNECTION_LOST:
        case WL_DISCONNECTED:
            return (int) WifiState::NOT_CONNECTED;
        case WL_CONNECTED:
            return (int) WifiState::CONNECTED;
        default:
            return (int) WifiState::CONNECTING;
    }
}

int Wifi::get_ap_state() {
    bool enable_ap = wifi_config_in_use.get("enable_soft_ap")->asBool();
    bool ap_fallback = wifi_config_in_use.get("ap_fallback_only")->asBool();
    if(!enable_ap)
        return 0;
    if(!ap_fallback)
        return 1;
    if(!soft_ap_running)
        return 2;

    return 3;
}

void Wifi::attempt_to_connect(int attempt) {
    if(WiFi.status() != WL_CONNECTED && attempt < 3) {
        Serial.print("Connecting to ");
        Serial.print(wifi_sta_config_in_use.get("ssid")->asString());
        Serial.print(" (");
        Serial.print(attempt + 1);
        Serial.print("/3)\n");
        WiFi.begin();

        task_scheduler.scheduleOnce("wifi_connect_attempt", [this, attempt](){
            attempt_to_connect(attempt + 1);
        }, 5000);
        return;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.printf("Failed to connect to %s.", "configured ssid");
        return;
    } else {
        Serial.println("");
        Serial.print("Connected to ");
        Serial.println(WiFi.SSID());
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }
}

void Wifi::connect_to_wifi()
{
    Serial.println("Wifi connecting");
    Serial.println();

    task_scheduler.scheduleOnce("wifi_connect", [this](){
        attempt_to_connect(0);
    }, 0);
}
