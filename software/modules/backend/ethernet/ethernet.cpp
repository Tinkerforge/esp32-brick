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

#include "ethernet.h"

#define ETH_ADDR        0
#define ETH_POWER_PIN   5
#define ETH_TYPE        ETH_PHY_KSZ8081

#include <ETH.h>

#include "task_scheduler.h"

extern TaskScheduler task_scheduler;

Ethernet::Ethernet()
{
    ethernet_config = Config::Object({
        {"enable_ethernet", Config::Bool(false)},
        {"hostname", Config::Str("wallbox", 32)},
        {"ip", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
        {"gateway", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
        {"subnet", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
        {"dns", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
        {"dns2", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
    });

    ethernet_state = Config::Object({
        {"connection_state", Config::Int(0)},
        {"ip", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
        {"full_duplex", Config::Bool(false)},
        {"link_speed", Config::Uint8(0)}
    });
}
/*
bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}*/

void Ethernet::setup()
{
    ethernet_config_in_use = ethernet_config;
    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("ETH Started");
            //set eth hostname here
            ETH.setHostname(ethernet_config_in_use.get("hostname")->asString().c_str());
        },
        ARDUINO_EVENT_ETH_START);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("ETH Connected");
        },
        ARDUINO_EVENT_ETH_CONNECTED);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("ETH MAC: %s, IPv4: %s, %s Duplex, %u Mbps", ETH.macAddress().c_str(), ETH.localIP().toString().c_str(), ETH.fullDuplex() ? "Full" : "Half", ETH.linkSpeed());
            //eth_connected = true;
        },
        ARDUINO_EVENT_ETH_GOT_IP);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("ETH Disconnected");
            //eth_connected = false;
        },
        ARDUINO_EVENT_ETH_DISCONNECTED);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
           logger.printfln("ETH Stopped");
            //eth_connected = false;
        },
        ARDUINO_EVENT_ETH_STOP);

    //WiFi.onEvent(WiFiEvent);

    /*
        #define ETH_ADDR        1
        #define ETH_POWER_PIN   5
        #define ETH_TYPE        ETH_PHY_IP101
    */
    //bool begin(uint8_t phy_addr=ETH_PHY_ADDR, int power=ETH_PHY_POWER, int mdc=ETH_PHY_MDC, int mdio=ETH_PHY_MDIO, eth_phy_type_t type=ETH_PHY_TYPE, eth_clock_mode_t clk_mode=ETH_CLK_MODE);
    task_scheduler.scheduleOnce("setup ethernet", [](){
        ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_TYPE);
    }, 10000);

    initialized = true;
}

void Ethernet::register_urls()
{

}

void Ethernet::loop()
{

}

EthernetState Ethernet::get_connection_state()
{
    return EthernetState::NOT_CONFIGURED;
}
