#include "tools.h"

#include "bindings/errors.h"

#include <Arduino.h>

bool deadline_elapsed(uint32_t deadline_ms) {
    uint32_t now = millis();

    if(now < deadline_ms) {
        uint32_t diff = deadline_ms - now;
        if (diff < UINT32_MAX / 2)
            return false;
        return true;
    }
    else {
        uint32_t diff = now - deadline_ms;
        if(diff > UINT32_MAX / 2)
            return false;
        return true;
    }
}

bool find_uid_by_did(TF_HalContext *hal, uint16_t device_id, char uid[7]) {
    char pos;
    uint16_t did;
    for (size_t i = 0; tf_hal_get_device_info(hal, i, uid, &pos, &did) == TF_E_OK; ++i) {
        if (did == device_id) {
            return true;
        }
    }
    return false;
}

bool send_event_allowed(AsyncEventSource *events) {
    // TODO: patch the library to get how many packets are waiting in the fullest client queue
    return events->count() > 0 && events->avgPacketsWaiting() < 8;
}