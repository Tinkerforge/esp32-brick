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

#pragma once

#include <PangolinMQTT.h>

#include "api.h"
#include "config.h"

#define MAX_CONNECT_ATTEMPT_INTERVAL_MS 5 * 60 * 1000

enum class MqttConnectionState {
    NOT_CONFIGURED,
    NOT_CONNECTED,
    CONNECTED,
    ERROR
};

struct MqttCommand {
    String topic;
    uint32_t max_len;
    std::function<void(String)> callback;
    bool forbid_retained;
};

class Mqtt : public IAPIBackend {
public:
    Mqtt();
    void setup();
    void register_urls();
    void loop();
    void connect();

    void publish(String topic_suffix, String payload);
    void subscribe(String topic_suffix, uint32_t max_payload_length, std::function<void(String)> callback, bool forbid_retained);

    //IAPIBackend implementation
    void addCommand(CommandRegistration reg);
    void addState(StateRegistration reg);
    void pushStateUpdate(String payload, String path);
    void wifiAvailable();

    bool initialized = false;

private:
    void apply_config();

    void onMqttError(uint8_t e,uint32_t info);
    void onMqttConnect(bool sessionPresent);
    void onMqttMessage(const char* topic, const uint8_t* payload, size_t len,uint8_t qos,bool retain,bool dup);
    void onMqttDisconnect(int8_t reason);


    Config mqtt_config;
    Config mqtt_state;

    Config mqtt_config_in_use;

    uint32_t connect_attempt_interval_ms;

    PangolinMQTT mqttClient;

    std::vector<MqttCommand> commands;
    bool was_connected = true;
};
