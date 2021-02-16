#pragma once

#include "api.h"

class Http : public IAPIBackend {
public:
    Http();
    void setup();
    void register_urls();
    void loop();

    //IAPIBackend implementation
    void addCommand(CommandRegistration reg);
    void addState(StateRegistration reg);
    void pushStateUpdate(String payload, String path);

    bool initialized = false;
};
