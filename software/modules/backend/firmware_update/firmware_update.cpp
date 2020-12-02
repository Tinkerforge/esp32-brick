#include "firmware_update.h"
#include "Arduino.h"

#include <Update.h>
#include <tools.h>

#include "modules/task_scheduler/task_scheduler.h"

extern AsyncWebServer server;
extern TaskScheduler task_scheduler;

FirmwareUpdate::FirmwareUpdate() {

}

void FirmwareUpdate::setup()
{

}

void handleUpdateChunk(int command, AsyncWebServerRequest *request, size_t index, uint8_t *data, size_t len, bool final) {
    if(index == 0 && !Update.begin(UPDATE_SIZE_UNKNOWN, command)) {
        request->send(400, "text/html", Update.errorString());
        return;
    }

    if(Update.write(data, len) != len) {
        request->send(400, "text/html", String("Failed to write with length") + len + ": " + Update.errorString());
        return;
    }

    if(final) {
        if(!Update.end(true)) {
            request->send(400, "text/html", String("Failed to finish writing update: ") + Update.errorString());
            return;
        }
    }
}

void FirmwareUpdate::register_urls()
{
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", "<form method='POST' action='/flash_firmware' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form> <form method='POST' action='/flash_spiffs' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update SPIFFS'></form>");
    });

    server.on("/flash_firmware", HTTP_POST, [this](AsyncWebServerRequest *request){
        AsyncWebServerResponse *response = request->beginResponse(Update.hasError() ? 400: 200, "text/plain", Update.hasError() ? Update.errorString() : "Update OK");
        response->addHeader("Connection", "close");

        if(!Update.hasError()) {
            task_scheduler.scheduleOnce("flash_firmware_reboot", [](){ESP.restart();}, 3000);
        }

        request->send(response);
    },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
        handleUpdateChunk(U_FLASH, request, index, data, len, final);
    });

    server.on("/flash_spiffs", HTTP_POST, [this](AsyncWebServerRequest *request){
        AsyncWebServerResponse *response = request->beginResponse(Update.hasError() ? 400: 200, "text/plain", Update.hasError() ? Update.errorString() : "Update OK");
        response->addHeader("Connection", "close");

        if(!Update.hasError()) {
            task_scheduler.scheduleOnce("flash_spiffs_reboot", [](){ESP.restart();}, 3000);
        }

        request->send(response);
    },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
        handleUpdateChunk(U_SPIFFS, request, index, data, len, final);
    });

    server.onFileUpload([](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
        handleUpdateChunk(filename == "spiffs" ? U_SPIFFS : U_FLASH, request, index, data, len, final);
    });
}

void FirmwareUpdate::onEventConnect(AsyncEventSourceClient *client)
{

}

void FirmwareUpdate::loop()
{

}
