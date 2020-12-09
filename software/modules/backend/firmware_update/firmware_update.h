#pragma once

#include "ESPAsyncWebServer.h"

class FirmwareUpdate {
public:
    FirmwareUpdate();
    void setup();
    void register_urls();
    void onEventConnect(AsyncEventSourceClient *client);
    void loop();

    bool initialized = false;
};
