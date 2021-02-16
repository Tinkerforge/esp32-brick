#include "http.h"

#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"

#include "api.h"
#include "task_scheduler.h"

extern API api;
extern AsyncWebServer server;
extern TaskScheduler task_scheduler;

Http::Http()
{
    api.registerBackend(this);
}

void Http::setup()
{

}

void Http::register_urls()
{

}

void Http::loop()
{

}

void Http::addCommand(CommandRegistration reg)
{
    AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler(String("/") + reg.path, [reg](AsyncWebServerRequest *request, JsonVariant &json){
        String message = reg.config->update_from_json(json);

        task_scheduler.scheduleOnce((String("notify command update for ") + reg.path).c_str(), [reg](){reg.callback();}, 0);

        if (message == "") {
            request->send(200, "text/html", "");
        } else {
            request->send(400, "text/html", message);
        }
    });
    server.addHandler(handler);
}

void Http::addState(StateRegistration reg)
{
    server.on((String("/") + reg.path).c_str(), HTTP_GET, [reg](AsyncWebServerRequest *request) {
        auto *response = request->beginResponseStream("application/json; charset=utf-8");
        reg.config->write_to_stream_except(*response, reg.keys_to_censor);
        request->send(response);
    });
}

void Http::pushStateUpdate(String payload, String path)
{

}
