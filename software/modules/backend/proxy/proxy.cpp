#include "proxy.h"

#include <AsyncJson.h>

#include "bindings/display_names.h"
#include "bindings/errors.h"
#include "bindings/hal_common.h"

#include "task_scheduler.h"

#include "api.h"

extern TF_HalContext hal;
extern AsyncWebServer server;
extern AsyncEventSource events;
extern TaskScheduler task_scheduler;

extern API api;

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
    api.addState("proxy/error_counters", &error_counters, {}, 1000);
    api.addState("proxy/devices", &devices, {}, 10000);

    task_scheduler.scheduleWithFixedDelay("update_error_counters", [this](){
        for(char c = 'A'; c <= 'F'; ++c) {
            uint32_t spitfp_checksum, spitfp_frame, tfp_frame, tfp_unexpected;

            tf_hal_get_error_counters(&hal, c, &spitfp_checksum, &spitfp_frame, &tfp_frame, &tfp_unexpected);

            error_counters.get(String(c))->get("SpiTfpChecksum")->updateUint(spitfp_checksum);
            error_counters.get(String(c))->get("SpiTfpFrame")->updateUint(spitfp_frame);
            error_counters.get(String(c))->get("TfpFrame")->updateUint(tfp_frame);
            error_counters.get(String(c))->get("TfpUnexpected")->updateUint(tfp_unexpected);
        }
    }, 5000, 5000);
}

void Proxy::onEventConnect(AsyncEventSourceClient *client)
{

}

void Proxy::loop()
{

}
