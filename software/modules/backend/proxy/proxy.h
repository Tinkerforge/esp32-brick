#pragma once

#include "ESPAsyncWebServer.h"

#include "net_arduino_esp32/net_arduino_esp32.h"
#include "config.h"

class Proxy {
public:
    Proxy();
    void setup();
    void register_urls();
    void onEventConnect(AsyncEventSourceClient *client);
    void loop();

private:
    TF_NetContext net;
    Config devices;
    Config error_counters;
};
