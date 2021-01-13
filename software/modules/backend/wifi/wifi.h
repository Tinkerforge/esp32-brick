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

    bool initialized = false;

private:
    void apply_soft_ap_config_and_start();
    void apply_sta_config_and_connect();

    int get_connection_state();
    int get_ap_state();

    WifiState state;

    Config wifi_ap_config;
    Config wifi_sta_config;
    Config wifi_state;

    Config wifi_scan_config;

    Config wifi_ap_config_in_use;
    Config wifi_sta_config_in_use;

    bool soft_ap_running = false;
    uint32_t connect_attempt_interval_ms;
};
