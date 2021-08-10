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

#include "esp32_ethernet_brick.h"
#include "Arduino.h"

#include "tools.h"
#include "hal_arduino_esp32_ethernet/hal_arduino_esp32_ethernet.h"

#define GREEN_LED 2
#define BLUE_LED 15
#define BUTTON 0

TF_HalContext hal;
extern uint32_t uid_numeric;
extern char uid[7];
extern char passphrase[20];
extern int8_t blue_led_pin;
extern int8_t green_led_pin;
extern int8_t button_pin;
extern bool factory_reset_requested;

TF_Port ports[6] = {{
        .port_name = 'A'
    }, {
        .port_name = 'B'
    }, {
        .port_name = 'C'
    }, {
        .port_name = 'D'
    }, {
        .port_name = 'E'
    }, {
        .port_name = 'F'
    }
};

ESP32EthernetBrick::ESP32EthernetBrick()
{

}

void ESP32EthernetBrick::setup()
{
    check(tf_hal_create(&hal, ports, sizeof(ports)/sizeof(ports[0])), "hal create");
    tf_hal_set_timeout(&hal, 100000);

    read_efuses(&uid_numeric, uid, passphrase);

    pinMode(GREEN_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);
    pinMode(BUTTON, INPUT);

    green_led_pin = GREEN_LED;
    blue_led_pin = BLUE_LED;
    button_pin = BUTTON;

}

void ESP32EthernetBrick::register_urls()
{

}

static uint8_t factory_reset_stage = 0;
static uint32_t last_led_toggle = 0;
static bool last_led_state = false;
static uint32_t last_btn_toggle = 0;
static bool last_btn_state = false;
static uint32_t last_state_change = 0;

bool wasPressedForXms(uint32_t ms) {
    return deadline_elapsed(last_btn_toggle + ms) && !last_btn_state;
}

bool wasToggledInLastXms(uint32_t ms) {
    return !deadline_elapsed(last_btn_toggle + ms);
}


void ledBlink(int8_t led_pin, int interval, int blinks_per_interval, int off_time_ms) {
    int t_in_second = millis() % interval;
    if (off_time_ms != 0 && (interval - t_in_second <= off_time_ms)) {
        digitalWrite(led_pin, 1);
        return;
    }

    //We want blinks_per_interval blinks and blinks_per_interval pauses between them. The off_time counts as pause.
    int state_count = ((2 * blinks_per_interval) - (off_time_ms != 0 ? 1 : 0));
    int state_interval = (interval - off_time_ms) / state_count;
    bool led = (t_in_second / state_interval) % 2 != 0;

    digitalWrite(led_pin, led);
}

/*
To trigger a factory reset, the user has to hold
the button down for three seconds (until the blue led blinks fast),
then release the button for three seconds (until the led stops blinking)
and repeat this process two times.

This has to be so complicated, because the ethernet chip
sends a 50 MHz clock to the ESP over pin 0, but while it is
disabled, it pulls the pin to ground. The chip is disabled
while the ESP starts to make sure the 50 MHz clock does not trigger
the ESP's download mode. (Pin 0 is also a strapping pin for the ESP)
*/
void ESP32EthernetBrick::loop()
{
    if (factory_reset_requested)
        return;

    bool btn = digitalRead(BUTTON);

    if (last_btn_state != btn) {
        last_btn_state = btn;
        last_btn_toggle = millis();
    }

    switch(factory_reset_stage) {
        case 0:
            ledBlink(BLUE_LED, 2000, 1, 0);

            if (wasPressedForXms(3000)) {
                factory_reset_stage = 1;
                last_state_change = millis();
            }
            break;
        case 1:
            ledBlink(BLUE_LED, 1000, 10, 0);

            if (deadline_elapsed(last_state_change + 3000) && wasToggledInLastXms(3000)) {
                factory_reset_stage = 2;
                last_state_change = millis();
            }
            break;
        case 2:
            digitalWrite(BLUE_LED, true);

            if (wasPressedForXms(3000)) {
                factory_reset_stage = 3;
                last_state_change = millis();
            }
            break;
        case 3:
            ledBlink(BLUE_LED, 1000, 10, 0);

            if (deadline_elapsed(last_state_change + 3000) && wasToggledInLastXms(3000)) {
                factory_reset_stage = 4;
                last_state_change = millis();
            }
            break;
        case 4:
            digitalWrite(BLUE_LED, true);

            if (wasPressedForXms(3000)) {
                digitalWrite(BLUE_LED, false);
                printf("Factory_reset!");
                factory_reset_stage = 0;
                last_state_change = millis();
                factory_reset_requested = true;
            }
            break;
    }

    if (deadline_elapsed(last_state_change + 10000)) {
        last_state_change = millis();
        factory_reset_stage = 0;
    }

    //digitalWrite(GREEN_LED, digitalRead(4));
}
