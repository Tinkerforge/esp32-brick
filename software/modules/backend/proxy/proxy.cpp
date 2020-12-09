#include "proxy.h"

#include <AsyncJson.h>

#include "bindings/display_names.h"
#include "bindings/errors.h"
#include "bindings/hal_common.h"

#include "modules/task_scheduler/task_scheduler.h"

extern TF_HalContext hal;
extern AsyncWebServer server;
extern AsyncEventSource events;
extern TaskScheduler task_scheduler;

Proxy::Proxy()
{
    devices = Config::Array(
        {},
        Config::Object({
            {"uid", Config::Str("", 7)},
            {"port", Config::Str("", 1)},
            {"name", Config::Str("", 50)},
        })
    );

    error_counters = Config::Object({
        {"A", Config::Object({
                {"SpiTfpChecksum", Config::Uint32(0)},
                {"SpiTfpFrame", Config::Uint32(0)},
                {"TfpFrame", Config::Uint32(0)},
                {"TfpUnexpected", Config::Uint32(0)},
            })
        },
        {"B", Config::Object({
                {"SpiTfpChecksum", Config::Uint32(0)},
                {"SpiTfpFrame", Config::Uint32(0)},
                {"TfpFrame", Config::Uint32(0)},
                {"TfpUnexpected", Config::Uint32(0)},
            })
        },
        {"C", Config::Object({
                {"SpiTfpChecksum", Config::Uint32(0)},
                {"SpiTfpFrame", Config::Uint32(0)},
                {"TfpFrame", Config::Uint32(0)},
                {"TfpUnexpected", Config::Uint32(0)},
            })
        },
        {"D", Config::Object({
                {"SpiTfpChecksum", Config::Uint32(0)},
                {"SpiTfpFrame", Config::Uint32(0)},
                {"TfpFrame", Config::Uint32(0)},
                {"TfpUnexpected", Config::Uint32(0)},
            })
        },
        {"E", Config::Object({
                {"SpiTfpChecksum", Config::Uint32(0)},
                {"SpiTfpFrame", Config::Uint32(0)},
                {"TfpFrame", Config::Uint32(0)},
                {"TfpUnexpected", Config::Uint32(0)},
            })
        },
        {"F", Config::Object({
                {"SpiTfpChecksum", Config::Uint32(0)},
                {"SpiTfpFrame", Config::Uint32(0)},
                {"TfpFrame", Config::Uint32(0)},
                {"TfpUnexpected", Config::Uint32(0)},
            })
        }
    });
}

void Proxy::setup()
{
    tf_net_create(&net, NULL, 0, NULL);
    tf_hal_set_net(&hal, &net);

    int i = 0;
    char uid[7] = {0};
    char port_name;
    uint16_t device_id;
    while(tf_hal_get_device_info(&hal, i, uid, &port_name, &device_id) == TF_E_OK) {
        devices.add();

        devices.get(devices.count() - 1)->get("uid")->updateString(String(uid));
        devices.get(devices.count() - 1)->get("port")->updateString(String(port_name));
        devices.get(devices.count() - 1)->get("name")->updateString(String(tf_get_device_display_name(device_id)));
        ++i;
    }

    initialized = true;
}

void Proxy::register_urls()
{
    server.on("/error_counter", HTTP_GET, [this](AsyncWebServerRequest *request) {
        auto *response = request->beginResponseStream("application/json; charset=utf-8");
        error_counters.write_to_stream(*response);
        request->send(response);
    });

    server.on("/devices", HTTP_GET, [this](AsyncWebServerRequest *request) {
        auto *response = request->beginResponseStream("application/json; charset=utf-8");
        devices.write_to_stream(*response);
        request->send(response);
    });

    task_scheduler.scheduleWithFixedDelay("update_error_counters", [this](){
        bool send_event = false;
        for(char c = 'A'; c <= 'F'; ++c) {
            uint32_t spitfp_checksum, spitfp_frame, tfp_frame, tfp_unexpected;

            tf_hal_get_error_counters(&hal, c, &spitfp_checksum, &spitfp_frame, &tfp_frame, &tfp_unexpected);

            send_event |= error_counters.get(String(c))->get("SpiTfpChecksum")->updateUint(spitfp_checksum);
            send_event |= error_counters.get(String(c))->get("SpiTfpFrame")->updateUint(spitfp_checksum);
            send_event |= error_counters.get(String(c))->get("TfpFrame")->updateUint(spitfp_checksum);
            send_event |= error_counters.get(String(c))->get("TfpUnexpected")->updateUint(spitfp_checksum);
        }

        if(send_event && send_event_allowed(&events)) {
            events.send(error_counters.to_string().c_str(), "bricklet_error_counters", millis());
        }
    }, 5000, 5000);
}

void Proxy::onEventConnect(AsyncEventSourceClient *client)
{
    client->send(error_counters.to_string().c_str(), "bricklet_error_counters", millis(), 1000);
    client->send(devices.to_string().c_str(), "devices", millis(), 1000);
}

void Proxy::loop()
{

}
