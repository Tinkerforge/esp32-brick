/* esp32-brick
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "firmware_update.h"
#include "Arduino.h"

#include <Update.h>
#include <SPIFFS.h>

#include "event_log.h"
#include "task_scheduler.h"
#include "tools.h"

extern EventLog logger;

extern AsyncWebServer server;
extern TaskScheduler task_scheduler;

extern bool firmware_update_allowed;

void factory_reset()
{
    for(int i = 0; i < 5; ++i) {
        digitalWrite(GREEN_LED, true);
        delay(200);
        digitalWrite(GREEN_LED, false);
        delay(200);
    }
    SPIFFS.end();
    SPIFFS.format();
    ESP.restart();
}

FirmwareUpdate::FirmwareUpdate() {

}

void FirmwareUpdate::setup()
{
    initialized = true;
}

void handleUpdateChunk(int command, AsyncWebServerRequest *request, size_t chunk_index, uint8_t *data, size_t chunk_length, bool final) {
    if(chunk_index == 0 && !Update.begin(UPDATE_SIZE_UNKNOWN, command)) {
        request->send(400, "text/html", Update.errorString());
        return;
    }

    // The firmware files are merged with the bootloader, partition table and slot configuration bins.
    // The bootloader starts at offset 0x1000, which is the first byte in the firmware file.
    // The first firmware slot (i.e. the one that is flashed over USB) starts at 0x10000.
    // So we have to skip the first 0x10000 - 0x1000 bytes, after them the actual firmware starts.
    const size_t firmware_offset = 0x10000 - 0x1000;
    if (chunk_index + chunk_length < firmware_offset) {
        return;
    }

    uint8_t *start = data;
    size_t length = chunk_length;

    if (chunk_index < firmware_offset) {
        size_t to_skip = firmware_offset - chunk_index;
        start += to_skip;
        length -= to_skip;
    }

    if(Update.write(start, length) != length) {
        request->send(400, "text/html", String("Failed to write with length") + length + ": " + Update.errorString());
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
        request->send(200, "text/html", "<form method='POST' action='/flash_firmware' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update Firmware'></form> <form method='POST' action='/flash_spiffs' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update SPIFFS'></form>");
    });

    server.on("/flash_firmware", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (!firmware_update_allowed) {
            AsyncWebServerResponse *response = request->beginResponse(400, "text/plain", "firmware_update.script.vehicle_connected");
            response->addHeader("Connection", "close");
            request->send(response);
            return;
        }

        AsyncWebServerResponse *response = request->beginResponse(Update.hasError() ? 400: 200, "text/plain", Update.hasError() ? Update.errorString() : "Update OK");
        response->addHeader("Connection", "close");

        if(!Update.hasError()) {
            task_scheduler.scheduleOnce("flash_firmware_reboot", [](){ESP.restart();}, 1000);
        }

        request->send(response);
    },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
        if (!firmware_update_allowed) {
            if(final) {
                AsyncWebServerResponse *response = request->beginResponse(413, "text/plain", "firmware_update.script.vehicle_connected");
                response->addHeader("Connection", "close");
                request->send(response);
            }
            return;
        }
        handleUpdateChunk(U_FLASH, request, index, data, len, final);
    });

    server.on("/flash_spiffs", HTTP_POST, [this](AsyncWebServerRequest *request){
        AsyncWebServerResponse *response = request->beginResponse(Update.hasError() ? 400: 200, "text/plain", Update.hasError() ? Update.errorString() : "Update OK");
        response->addHeader("Connection", "close");

        if(!Update.hasError()) {
            task_scheduler.scheduleOnce("flash_spiffs_reboot", [](){ESP.restart();}, 1000);
        }

        request->send(response);
    },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
        handleUpdateChunk(U_SPIFFS, request, index, data, len, final);
    });

    AsyncCallbackJsonWebHandler *factory_reset_handler = new AsyncCallbackJsonWebHandler("/factory_reset", [this](AsyncWebServerRequest *request, JsonVariant &json){
        if(!json["do_i_know_what_i_am_doing"].is<bool>()) {
            request->send(400, "text/html", "you don't seem to know what you are doing");
            return;
        }

        if(json["do_i_know_what_i_am_doing"].as<bool>()) {
            task_scheduler.scheduleOnce("factory_reset", [](){
                logger.printfln("Factory reset requested");
                factory_reset();
            }, 3000);
            request->send(200, "text/html", "Factory reset initiated");
        } else {
            request->send(400, "text/html", "Factory reset NOT initiated");
        }
    });
    server.addHandler(factory_reset_handler);
}

void FirmwareUpdate::loop()
{
    bool btn0 = digitalRead(0);

    if(btn0 != last_btn_value) {
        last_btn_change = millis();
    }

    last_btn_value = btn0;

    if(!btn0 && deadline_elapsed(last_btn_change + 10000)) {
        logger.printfln("IO0 button was pressed for 10 seconds. Resetting to factory defaults.");
        factory_reset();
    }
}
