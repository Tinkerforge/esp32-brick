#pragma once

#include "ESPAsyncWebServer.h"
#include "ArduinoJson.h"
#include "config.h"

#define MAX_CONNECT_ATTEMPT_INTERVAL_MS 5 * 60 * 1000

enum class WifiState {
    NOT_CONNECTED,
    CONNECTING,
    CONNECTED
};

class Wifi {
public:
    Wifi();
    void setup();
    void register_urls();
    void onEventConnect(AsyncEventSourceClient *client);
    void loop();

private:
    void apply_soft_ap_config_and_start();
    void apply_sta_config_and_connect();
    int read_config_from_flash();
    int write_config_to_flash();
    bool read_wifi_configuration(JsonVariant json, String &message);

    int get_connection_state();
    int get_ap_state();

    WifiState state;

    Config wifi_config;
    Config wifi_soft_ap_config;
    Config wifi_sta_config;
    Config wifi_state;

    Config wifi_config_in_use;
    Config wifi_soft_ap_config_in_use;
    Config wifi_sta_config_in_use;

    bool soft_ap_running = false;
    uint32_t connect_attempt_interval_ms;
};
