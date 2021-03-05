#include "authentication.h"

#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

#include "api.h"
#include "event_log.h"

extern AsyncWebServer server;
extern API api;
extern EventLog logger;

Authentication::Authentication()
{
    authentication_config = Config::Object({
        {"enable_auth", Config::Bool(false)},
        {"username", Config::Str("", 64)},
        {"password", Config::Str("", 64)},
    });
}

void Authentication::setup()
{
    if(SPIFFS.exists("/authentication_config.json")) {
        File file = SPIFFS.open("/authentication_config.json");
        String error = authentication_config.update_from_file(file);
        file.close();
        if(error != "")
            logger.printfln(error.c_str());
    }

    if (authentication_config.get("enable_auth")->asBool()) {
        String user = authentication_config.get("username")->asString();
        String pass = authentication_config.get("password")->asString();

        server.setAuthentication(user.c_str(), pass.c_str());
        logger.printfln("Web interface authentication enabled.");
    }

    initialized = true;
}

void Authentication::register_urls()
{
    api.addPersistentConfig("authentication/config", &authentication_config, {"password"}, 10000);

    auto &handler = server.on("/logged_out", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "Logged out.");
    });
    handler.setAuthentication("", "");
}

void Authentication::loop()
{

}
