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

        if (message == "") {
            task_scheduler.scheduleOnce((String("notify command update for ") + reg.path).c_str(), [reg](){reg.callback();}, 0);
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

void Http::wifiAvailable()
{

}
