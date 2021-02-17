#include "wifi.h"

#include <ESPmDNS.h>
#include <SPIFFS.h>

#include <esp_wifi.h>

#include "ArduinoJson.h"
#include "AsyncJson.h"

#include "task_scheduler.h"
#include "tools.h"
#include "api.h"
#include "event_log.h"

extern EventLog logger;

#define RECONNECT_TIMEOUT_MS 30000

extern TaskScheduler task_scheduler;
extern AsyncWebServer server;
extern char uid[7];
extern char passphrase[20];

extern API api;

Wifi::Wifi() {
    wifi_ap_config = Config::Object({
        {"enable_ap", Config::Bool(true)},
        {"ap_fallback_only", Config::Bool(false)},
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
        {"enable_sta", Config::Bool(false)},
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
        {"ap_state", Config::Int(0)},
        {"ap_bssid", Config::Str("", 20)},
        {"sta_ip", Config::Array({
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
        {"sta_rssi", Config::Int8(0)}
    });

    wifi_scan_config = Config::Null();
}

void Wifi::apply_soft_ap_config_and_start() {
    uint8_t ip[4];
    uint8_t gateway[4];
    uint8_t subnet[4];

    wifi_ap_config_in_use.get("ip")->fillArray<uint8_t, Config::ConfUint>(ip, 4);
    wifi_ap_config_in_use.get("gateway")->fillArray<uint8_t, Config::ConfUint>(gateway, 4);
    wifi_ap_config_in_use.get("subnet")->fillArray<uint8_t, Config::ConfUint>(subnet, 4);

    int counter = 0;
    while((ip[0] != WiFi.softAPIP()[0]) || (ip[1] != WiFi.softAPIP()[1]) || (ip[2] != WiFi.softAPIP()[2]) || (ip[3] != WiFi.softAPIP()[3])) {
        WiFi.softAPConfig(ip, gateway, subnet);
        ++counter;
    }
    logger.printfln("Had to configure softAP ip %d times.", counter);
    delay(2000);

    logger.printfln("Soft AP started.");
    logger.printfln("    SSID: %s", wifi_ap_config_in_use.get("ssid")->asString().c_str());
    Serial.printf("    passphrase: %s\n", wifi_ap_config_in_use.get("passphrase")->asString().c_str());
    logger.printfln("    hostname: %s", wifi_ap_config_in_use.get("hostname")->asString().c_str());

    WiFi.softAPsetHostname(wifi_ap_config_in_use.get("hostname")->asString().c_str());

    WiFi.softAP(wifi_ap_config_in_use.get("ssid")->asString().c_str(),
                wifi_ap_config_in_use.get("passphrase")->asString().c_str(),
                wifi_ap_config_in_use.get("channel")->asUint(),
                wifi_ap_config_in_use.get("hide_ssid")->asBool());

    soft_ap_running = true;
    IPAddress myIP = WiFi.softAPIP();
    logger.printfln("    IP: %u.%u.%u.%u", myIP[0], myIP[1], myIP[2], myIP[3]);
}

void Wifi::apply_sta_config_and_connect() {
    if(this->state == WifiState::CONNECTED) {
        return;
    }

    this->state = WifiState::CONNECTING;

    WiFi.persistent(false);
    WiFi.setAutoReconnect(false);
    WiFi.disconnect(false, true);

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


    WiFi.begin(ssid.c_str(), passphrase.c_str(), 0, bssid_lock ? bssid : nullptr, false);

    if(ip != 0) {
        WiFi.config(ip, gateway, subnet, dns, dns2);
    } else {
        WiFi.config((uint32_t)0, (uint32_t)0, (uint32_t)0);
    }

    WiFi.setHostname(wifi_sta_config_in_use.get("hostname")->asString().c_str());

    logger.printfln("Connecting to %s", wifi_sta_config_in_use.get("ssid")->asString().c_str());

    WiFi.begin(ssid.c_str(), passphrase.c_str(), 0, bssid_lock ? bssid : nullptr, true);
}

void Wifi::setup()
{
    String default_hostname = String(__HOST_PREFIX__) + String("-") + String(uid);
    String default_passphrase = String(passphrase);

    WiFi.persistent(false);

    WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
            if(this->state == WifiState::CONNECTING) {
                logger.printfln("Failed to connect to %s", wifi_sta_config_in_use.get("ssid")->asString().c_str());
            } else if (this->state == WifiState::CONNECTED) {
                logger.printfln("Disconnected from %s", wifi_sta_config_in_use.get("ssid")->asString().c_str());
            }

            this->state = WifiState::NOT_CONNECTED;

            connect_attempt_interval_ms = MIN(connect_attempt_interval_ms * 2, MAX_CONNECT_ATTEMPT_INTERVAL_MS);

            logger.printfln(" next attempt in %u seconds.", connect_attempt_interval_ms / 1000);

            task_scheduler.scheduleOnce("wifi_connect", [this](){
                apply_sta_config_and_connect();
            }, connect_attempt_interval_ms);
        },
        SYSTEM_EVENT_STA_DISCONNECTED);

    WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
            this->state = WifiState::CONNECTED;

            logger.printfln("Connected to %s", WiFi.SSID().c_str());
            connect_attempt_interval_ms = 5000;
        },
        SYSTEM_EVENT_STA_CONNECTED);

    WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
            // Sometimes the SYSTEM_EVENT_STA_CONNECTED is not fired.
            // Instead we get the SYSTEM_EVENT_STA_GOT_IP twice?
            // Make sure that the state is set to connected here,
            // or else MQTT will never attempt to connect.
            this->state = WifiState::CONNECTED;

            auto ip = WiFi.localIP();
            logger.printfln("Got IP address: %u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
            wifi_state.get("sta_ip")->get(0)->updateUint(ip[0]);
            wifi_state.get("sta_ip")->get(1)->updateUint(ip[1]);
            wifi_state.get("sta_ip")->get(2)->updateUint(ip[2]);
            wifi_state.get("sta_ip")->get(3)->updateUint(ip[3]);
        },
        SYSTEM_EVENT_STA_GOT_IP);

    WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
        if(this->state != WifiState::CONNECTED)
            return;
        logger.printfln("Lost IP. Forcing disconnect and reconnect of WiFi");
        wifi_state.get("sta_ip")->get(0)->updateUint(0);
        wifi_state.get("sta_ip")->get(1)->updateUint(0);
        wifi_state.get("sta_ip")->get(2)->updateUint(0);
        wifi_state.get("sta_ip")->get(3)->updateUint(0);
        this->state = WifiState::NOT_CONNECTED;
        WiFi.disconnect(false, true);
    }, SYSTEM_EVENT_STA_LOST_IP);

    connect_attempt_interval_ms = 5000;

    if(SPIFFS.exists("/wifi_sta_config.json")) {
        File file = SPIFFS.open("/wifi_sta_config.json");
        String error = wifi_sta_config.update_from_file(file);
        file.close();
        if(error != "")
            logger.printfln(error.c_str());
    } else {
        wifi_sta_config.get("hostname")->updateString(default_hostname);
    }

    if(SPIFFS.exists("/wifi_ap_config.json")) {
        File file = SPIFFS.open("/wifi_ap_config.json");
        String error = wifi_ap_config.update_from_file(file);
        file.close();
        if(error != "")
            logger.printfln(error.c_str());
    } else {
        wifi_ap_config.get("hostname")->updateString(default_hostname);
        wifi_ap_config.get("ssid")->updateString(default_hostname);
        wifi_ap_config.get("passphrase")->updateString(default_passphrase);
        File file = SPIFFS.open("/wifi_ap_config.json", "w");
        wifi_ap_config.save_to_file(file);
        file.close();
    }

    wifi_ap_config_in_use = wifi_ap_config;
    wifi_sta_config_in_use = wifi_sta_config;

    bool enable_ap = wifi_ap_config_in_use.get("enable_ap")->asBool();
    bool enable_sta = wifi_sta_config_in_use.get("enable_sta")->asBool();
    bool ap_fallback_only = wifi_ap_config_in_use.get("ap_fallback_only")->asBool();

    if (enable_sta && enable_ap) {
        WiFi.mode(WIFI_AP_STA);
    } else if (enable_ap) {
        WiFi.mode(WIFI_AP);
    } else if (enable_sta) {
        WiFi.mode(WIFI_STA);
    } else {
        WiFi.mode(WIFI_OFF);
    }

    wifi_country_t config;
    config.cc[0] = 'D';
    config.cc[1] = 'E';
    config.cc[2] = ' ';
    config.schan = 1;
    config.nchan = 13;
    config.policy = WIFI_COUNTRY_POLICY_AUTO;
    esp_wifi_set_country(&config);

    esp_wifi_set_ps(WIFI_PS_NONE);

    WiFi.setTxPower(WIFI_POWER_19_5dBm);

    wifi_state.get("ap_bssid")->updateString(WiFi.softAPmacAddress());

    if (enable_ap && !ap_fallback_only) {
        apply_soft_ap_config_and_start();
    } else {
        WiFi.softAPdisconnect(true);
    }

    if (enable_sta) {
        apply_sta_config_and_connect();
    }

    /*use mdns for host name resolution*/
    if (!MDNS.begin(wifi_ap_config_in_use.get("hostname")->asString().c_str())) {
        logger.printfln("Error setting up mDNS responder!");
    } else {
        logger.printfln("mDNS responder started");
    }

    task_scheduler.scheduleWithFixedDelay("wifi_rssi", [this](){
        wifi_state.get("sta_rssi")->updateInt(WiFi.RSSI());
    }, 5000, 5000);

    initialized = true;
}

void Wifi::register_urls()
{
    api.addState("wifi/state", &wifi_state, {}, 1000);

    api.addCommand("wifi/scan", &wifi_scan_config, {}, [](){
        logger.printfln("Scanning for wifis...");
        WiFi.scanDelete();

        // WIFI_SCAN_FAILED also means the scan is done.
        if(WiFi.scanComplete() == WIFI_SCAN_FAILED){
            WiFi.scanNetworks(true, true);
        }
    });

    server.on("/wifi/scan_results", HTTP_GET, [](AsyncWebServerRequest *request) {
        int network_count = WiFi.scanComplete();

        logger.printfln("scan done");

        //result line: {"ssid": "%s", "bssid": "%s", "rssi": %d, "channel": %d, "encryption": %d}
        //worst case length ~ 140

        if (network_count == 0) {
            request->send(200, "application/json; charset=utf-8", "[]");
        } else {
            String result;
            result.reserve(145 * network_count);
            logger.printfln("%d networks found", network_count);
            result += "[";

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
            result += "]";
            request->send(200, "application/json; charset=utf-8", result);
        }
    });

    api.addPersistentConfig("wifi/sta_config", &wifi_sta_config, {"passphrase"}, 1000);
    api.addPersistentConfig("wifi/ap_config", &wifi_ap_config, {"passphrase"}, 1000);
}

void Wifi::loop()
{
    wifi_state.get("connection_state")->updateInt(get_connection_state());
    wifi_state.get("ap_state")->updateInt(get_ap_state());

    if (wifi_sta_config_in_use.get("enable_sta")->asBool() &&
        wifi_ap_config_in_use.get("enable_ap")->asBool() &&
        wifi_ap_config_in_use.get("ap_fallback_only")->asBool() &&
        WiFi.status() != WL_CONNECTED &&
        !soft_ap_running) {
            apply_soft_ap_config_and_start();
    }

    if (wifi_sta_config_in_use.get("enable_sta")->asBool() &&
        wifi_ap_config_in_use.get("ap_fallback_only")->asBool() &&
        WiFi.status() == WL_CONNECTED &&
        soft_ap_running) {
        logger.printfln("Wifi connected. Stopping soft AP");
        WiFi.softAPdisconnect(true);
        soft_ap_running = false;
    }
}

int Wifi::get_connection_state() {
    if (!wifi_sta_config_in_use.get("enable_sta")->asBool())
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
    bool enable_ap = wifi_ap_config_in_use.get("enable_ap")->asBool();
    bool ap_fallback = wifi_ap_config_in_use.get("ap_fallback_only")->asBool();
    if(!enable_ap)
        return 0;
    if(!ap_fallback)
        return 1;
    if(!soft_ap_running)
        return 2;

    return 3;
}
