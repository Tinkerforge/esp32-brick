#include "sse.h"

#include "ESPAsyncWebServer.h"

#include "api.h"

extern API api;
extern AsyncWebServer server;

Sse::Sse() : events("/events") {
    api.registerBackend(this);
}

void Sse::setup() {
    events.onConnect([](AsyncEventSourceClient *client){
        if(client->lastId()){
            logger.printfln("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
        }

        for(auto &reg : api.states) {
            client->send(reg.config->to_string_except(reg.keys_to_censor).c_str(), reg.path.c_str(), millis(), 1000);
        }
    });

    server.addHandler(&events);
}

void Sse::register_urls() {

}

void Sse::loop() {

}

void Sse::addCommand(CommandRegistration reg) {

}

void Sse::addState(StateRegistration reg) {

}

void Sse::pushStateUpdate(String payload, String path) {
    events.send(payload.c_str(), path.c_str(), millis());
}
