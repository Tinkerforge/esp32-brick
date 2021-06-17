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

#include "hidden_proxy.h"

#include "bindings/hal_common.h"

extern TF_HalContext hal;
extern AsyncWebServer server;

HiddenProxy::HiddenProxy()
{

}

void HiddenProxy::setup()
{

}

void HiddenProxy::register_urls()
{
    server.on("/hidden_proxy/enable", [this](AsyncWebServerRequest *request) {
        tf_hal_set_net(&hal, NULL);
        tf_net_create(&net, NULL, 0, NULL);
        tf_hal_set_net(&hal, &net);
        request->send(200);
    });

    server.on("/hidden_proxy/disable", [this](AsyncWebServerRequest *request) {
        tf_hal_set_net(&hal, NULL);
        tf_net_destroy(&net);
        request->send(200);
    });
}

void HiddenProxy::loop()
{

}
