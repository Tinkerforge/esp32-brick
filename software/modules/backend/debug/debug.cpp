#include "debug.h"

#include <Arduino.h>

#include "ArduinoJson.h"
#include "AsyncJson.h"

#include "modules/task_scheduler/task_scheduler.h"

#include "tools.h"
#include "api.h"


extern TaskScheduler task_scheduler;
extern AsyncWebServer server;
extern AsyncEventSource events;
extern char uid[7];
extern API api;

Debug::Debug() {

}

void Debug::setup()
{
    debug_state = Config::Object({
        {"uptime", Config::Uint32(0)},
        {"free_heap", Config::Uint32(0)},
        {"largest_free_heap_block", Config::Uint32(0)}
    });

    task_scheduler.scheduleWithFixedDelay("update_debug_state", [this](){
        debug_state.get("uptime")->updateUint(millis());
        debug_state.get("free_heap")->updateUint(ESP.getFreeHeap());
        debug_state.get("largest_free_heap_block")->updateUint(ESP.getMaxAllocHeap());
    }, 1000, 1000);

    initialized = true;
}

void Debug::register_urls()
{
    api.addState("debug/state", &debug_state, {}, 1000);
}

void Debug::onEventConnect(AsyncEventSourceClient *client)
{

}

void Debug::loop()
{

}
