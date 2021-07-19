#include "ws.h"

#include <esp_http_server.h>

#include "keep_alive.h"
#include "task_scheduler.h"
#include "web_server.h"

extern TaskScheduler task_scheduler;
extern WebServer server;
extern API api;

WS::WS() : web_sockets()
{
    api.registerBackend(this);
}

void WS::setup()
{

}

void WS::register_urls()
{
    web_sockets.onConnect([this](WebSocketsClient client){
        String to_send = "";
        for(auto &reg : api.states) {
            to_send += String("{\"topic\":\"") + reg.path + String("\",\"payload\":") + reg.config->to_string_except(reg.keys_to_censor) + String("}\n");
        }
        client.send(to_send.c_str(), to_send.length());
    });

    web_sockets.start("/ws");

    task_scheduler.scheduleWithFixedDelay("ws_keep_alive", [this](){
        const char *payload = "{\"topic\": \"keep-alive\", \"payload\": \"null\"}\n";
        web_sockets.sendToAll(payload, strlen(payload));
    }, 1000, 1000);
}

void WS::loop()
{

}

void WS::addCommand(CommandRegistration reg)
{

}

void WS::addState(StateRegistration reg)
{

}

void WS::pushStateUpdate(String payload, String path)
{
    String to_send = String("{\"topic\":\"") + path + String("\",\"payload\":") + payload + String("}\n");
    web_sockets.sendToAll(to_send.c_str(), to_send.length());
}

void WS::wifiAvailable()
{

}
