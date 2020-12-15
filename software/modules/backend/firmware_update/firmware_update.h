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

private:
    uint32_t last_btn_change = 0;
    bool last_btn_value = false;
};
