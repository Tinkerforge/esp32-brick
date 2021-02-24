#pragma once

#include "ESPAsyncWebServer.h"

#include "api.h"
#include "config.h"

class Sse : public IAPIBackend {
public:
    Sse();
    void setup();
    void register_urls();
    void loop();

    //IAPIBackend implementation
    void addCommand(CommandRegistration reg);
    void addState(StateRegistration reg);
    void pushStateUpdate(String payload, String path);
    void wifiAvailable();

    bool initialized = false;

    AsyncEventSource events;
};
