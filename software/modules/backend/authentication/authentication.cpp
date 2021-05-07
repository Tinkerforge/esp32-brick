#include "authentication.h"

#include "Arduino.h"

#include "ESPAsyncWebServer.h"

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
    api.restorePersistentConfig("authentication/config", &authentication_config);

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
        // Safari does not support an unauthenticated login page and an authenticated main page on the same url,
        // as it does not proactively send the credentials if the same url is known to have an unauthenticated
        // version.
        const String &user_agent = request->header("User-Agent");
        bool is_safari = user_agent.indexOf("Safari/") >= 0 && user_agent.indexOf("Version/") >= 0 && user_agent.indexOf("Chrome/") == -1 && user_agent.indexOf("Chromium/") == -1;
        if (is_safari) {
            request->requestAuthentication();
            return;
        }

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

    server.on("/credential_check", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Credentials okay");
    });

    auto &handler = server.on("/login_state", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Logged in");
    });

    handler.onNotAuthorized([](AsyncWebServerRequest *request) {
        // Same reasoning as above. If we don't force Safari, it does not send credentials, which breaks the login_state check.
        const String &user_agent = request->header("User-Agent");
        bool is_safari = user_agent.indexOf("Safari/") >= 0 && user_agent.indexOf("Version/") >= 0 && user_agent.indexOf("Chrome/") == -1 && user_agent.indexOf("Chromium/") == -1;
        if (is_safari) {
            request->requestAuthentication();
            return;
        }

        request->send(200, "text/plain", "Not logged in");
    });
}

void Authentication::loop()
{

}
