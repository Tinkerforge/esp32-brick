#pragma once

#include "ESPAsyncWebServer.h"
#include "ArduinoJson.h"

#include "config.h"

class Debug {
public:
    Debug();
    void setup();
    void register_urls();
    void onEventConnect(AsyncEventSourceClient *client);
    void loop();

    bool initialized = false;
private:
    Config debug_state;
};
