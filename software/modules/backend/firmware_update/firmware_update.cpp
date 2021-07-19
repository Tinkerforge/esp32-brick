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

extern WebServer server;
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

bool FirmwareUpdate::handleUpdateChunk(int command, WebServerRequest request, size_t chunk_index, uint8_t *data, size_t chunk_length, bool final, size_t complete_length) {
    // The firmware files are merged with the bootloader, partition table and slot configuration bins.
    // The bootloader starts at offset 0x1000, which is the first byte in the firmware file.
    // The first firmware slot (i.e. the one that is flashed over USB) starts at 0x10000.
    // So we have to skip the first 0x10000 - 0x1000 bytes, after them the actual firmware starts.
    // Don't skip anything if we flash the SPIFFS.
    const size_t firmware_offset = command == U_FLASH ? 0x10000 - 0x1000 : 0;

    if(chunk_index == 0 && !Update.begin(complete_length - firmware_offset, command)) {
        logger.printfln("Failed to start update: %s", Update.errorString());
        request.send(400, "text/plain", Update.errorString());
        this->firmware_update_running = false;
        return false;
    }

    if (chunk_index + chunk_length < firmware_offset) {
        return true;
    }

    uint8_t *start = data;
    size_t length = chunk_length;

    if (chunk_index < firmware_offset) {
        size_t to_skip = firmware_offset - chunk_index;
        start += to_skip;
        length -= to_skip;
    }

    auto written = Update.write(start, length);
    if(written != length) {
        logger.printfln("Failed to write update chunk with length %d; written %d, error: %s", length, written, Update.errorString());
        request.send(400, "text/plain", (String("Failed to write update: ") + Update.errorString()).c_str());
        this->firmware_update_running = false;
        return false;
    }

    if(final && !Update.end(true)) {
        logger.printfln("Failed to apply update: %s", Update.errorString());
        request.send(400, "text/plain", (String("Failed to apply update: ") + Update.errorString()).c_str());
        this->firmware_update_running = false;
        return false;
    }

    return true;
}

void FirmwareUpdate::register_urls()
{
    server.on("/update", HTTP_GET, [](WebServerRequest request){
        request.send(200, "text/html", "<form><input id=\"firmware\"type=\"file\"> <button id=\"u_firmware\"type=\"button\"onclick='u(\"firmware\")'>Upload firmware</button> <label id=\"p_firmware\"></label></form><form><input id=\"spiffs\"type=\"file\"> <button id=\"u_spiffs\"type=\"button\"onclick='u(\"spiffs\")'>Upload SPIFFS</button> <label id=\"p_spiffs\"></label></form><script>function u(e){var t,n,d,o=document.getElementById(e).files;0==o.length?alert(\"No file selected!\"):(document.getElementById(\"firmware\").disabled=!0,document.getElementById(\"u_firmware\").disabled=!0,document.getElementById(\"spiffs\").disabled=!0,document.getElementById(\"u_spiffs\").disabled=!0,t=o[0],n=new XMLHttpRequest,d=document.getElementById(\"p_\"+e),n.onreadystatechange=function(){4==n.readyState&&(200==n.status?(document.open(),document.write(n.responseText),document.close()):(0==n.status?alert(\"Server closed the connection abruptly!\"):alert(n.status+\" Error!\\n\"+n.responseText),location.reload()))},n.upload.addEventListener(\"progress\",function(e){e.lengthComputable&&(d.innerHTML=e.loaded/e.total*100+\"% (\"+e.loaded+\" / \"+e.total+\")\")},!1),n.open(\"POST\",\"/flash_\"+e,!0),n.send(t))}</script>");
    });

    server.on("/flash_firmware", HTTP_POST, [this](WebServerRequest request){
        this->firmware_update_running = false;
        if (!firmware_update_allowed) {
            request.send(423, "text/plain", "vehicle connected");
            return;
        }

        if(!Update.hasError()) {
            task_scheduler.scheduleOnce("flash_firmware_reboot", [](){ESP.restart();}, 1000);
        }

        request.send(Update.hasError() ? 400: 200, "text/plain", Update.hasError() ? Update.errorString() : "Update OK");
    },[this](WebServerRequest request, String filename, size_t index, uint8_t *data, size_t len, bool final){
        if (!firmware_update_allowed) {
            request.send(423, "text/plain", "vehicle connected");
            this->firmware_update_running = false;
            return false;
        }
        this->firmware_update_running = true;
        return handleUpdateChunk(U_FLASH, request, index, data, len, final, request.contentLength());
    });

    server.on("/flash_spiffs", HTTP_POST, [this](WebServerRequest request){
        if(!Update.hasError()) {
            task_scheduler.scheduleOnce("flash_spiffs_reboot", [](){ESP.restart();}, 1000);
        }

        request.send(Update.hasError() ? 400: 200, "text/plain", Update.hasError() ? Update.errorString() : "Update OK");
    },[this](WebServerRequest request, String filename, size_t index, uint8_t *data, size_t len, bool final){
        return handleUpdateChunk(U_SPIFFS, request, index, data, len, final, request.contentLength());
    });

    server.on("/factory_reset", HTTP_PUT, [this](WebServerRequest request) {
        char *payload = request.receive();
        StaticJsonDocument<16> doc;

        DeserializationError error = deserializeJson(doc, payload, request.contentLength());

        if (error) {
            logger.printfln("Failed to parse command payload: %s", error.c_str());
            request.send(400);
            free(payload);
            return;
        }

        if(!doc["do_i_know_what_i_am_doing"].is<bool>()) {
            request.send(400, "text/html", "you don't seem to know what you are doing");
            free(payload);
            return;
        }

        if(doc["do_i_know_what_i_am_doing"].as<bool>()) {
            task_scheduler.scheduleOnce("factory_reset", [](){
                logger.printfln("Factory reset requested");
                factory_reset();
            }, 3000);
            request.send(200, "text/html", "Factory reset initiated");
        } else {
            request.send(400, "text/html", "Factory reset NOT initiated");
        }

        free(payload);
    });

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
