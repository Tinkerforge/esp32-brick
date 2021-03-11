#include "authentication.h"

#include "Arduino.h"

#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

#include "api.h"
#include "event_log.h"

#include "login.html.h"

extern AsyncWebServer server;
extern AsyncCallbackWebHandler *main_page_handler;
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

    main_page_handler->onNotAuthorized([](AsyncWebServerRequest *request) {
        auto *response = request->beginChunkedResponse("text/html", [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
            auto CHUNK_SIZE = 1024 * 10;
            //Write up to "maxLen" bytes into "buffer" and return the amount written.
            //index equals the amount of bytes that have been already sent
            //You will be asked for more data until 0 is returned
            //Keep in mind that you can not delay or yield waiting for more data!
            size_t to_write = MIN(MIN(CHUNK_SIZE, maxLen), login_html_gz_len - index);
            memcpy(buffer, login_html_gz + index, to_write);
            return to_write;
        });

        response->addHeader("Content-Encoding", "gzip");
        response->addHeader("ETag", String((uint32_t)(_BUILD_TIME_), 16));
        response->setCode(200);
        request->send(response);
    });

    server.on("/login_check", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "Logged in.");
    });
}

void Authentication::loop()
{

}
