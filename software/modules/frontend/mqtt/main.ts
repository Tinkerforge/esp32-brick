import $ from "jquery";

import * as util from "../util";

interface MqttConfig {
    enable_mqtt: boolean,
    broker_host: string,
    broker_port: number,
    broker_username: string,
    broker_password: string,
    global_topic_prefix: string
    client_name: string
}

interface MqttState {
    connection_state: number
    last_error: number
}

function update_mqtt_config(config: MqttConfig) {
    $('#mqtt_enable').prop("checked", config.enable_mqtt);
    $('#mqtt_broker_host').val(config.broker_host);
    $('#mqtt_broker_port').val(config.broker_port);
    $('#mqtt_broker_username').val(config.broker_username);
    // omit password
    $('#mqtt_topic_prefix').val(config.global_topic_prefix);
    $('#mqtt_client_name').val(config.client_name);
    if(config.enable_mqtt && config.broker_host != ""){
        $('#mqtt_status_broker_host').html(" " + __("mqtt.script.to") + " " + config.broker_host);
    } else {
        $('#mqtt_status_broker_host').html("");
    }
}

function save_mqtt_config() {
    let payload: MqttConfig = {
        enable_mqtt: $('#mqtt_enable').is(':checked'),
        broker_host: $('#mqtt_broker_host').val(),
        broker_port: parseInt($('#mqtt_broker_port').val(), 10),
        broker_username: $('#mqtt_broker_username').val(),
        broker_password: $('#mqtt_broker_password').val(),
        global_topic_prefix: $('#mqtt_topic_prefix').val(),
        client_name: $('#mqtt_client_name').val(),
    };

    $.ajax({
        url: '/mqtt_config_update',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(payload),
        success: $('#mqtt_reboot').modal('show'),
        error: (_x, _y, error) => util.show_alert("alert-danger", __("mqtt.script.save_failed"), error)
    });
}

function update_mqtt_state(state: MqttState) {
    util.update_button_group("btn_group_mqtt_state", state.connection_state);
    if(state.connection_state == 3) {
        $('#mqtt_status_error').html(" " + state.last_error);
    }
}

export function addEventListeners(source: EventSource) {
    source.addEventListener('mqtt_config', function (e: util.SSE) {
        update_mqtt_config(<MqttConfig>(JSON.parse(e.data)));
    }, false);

    source.addEventListener('mqtt_state', function (e: util.SSE) {
        update_mqtt_state(<MqttState>(JSON.parse(e.data)));
    }, false);
}

export function init() {
    console.log("init");
    let button = <HTMLButtonElement>document.getElementById("mqtt_show_password");
    button.addEventListener("change", util.toggle_password_fn("#mqtt_broker_password"));

    let form = <HTMLFormElement>$('#mqtt_config_form')[0];
    form.addEventListener('submit', function (event: Event) {
        form.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (form.checkValidity() === false) {
            return;
        }

        save_mqtt_config();
    }, false);

    (<HTMLButtonElement>document.getElementById("mqtt_reboot_button")).addEventListener("click", () => {
        $('#mqtt_reboot').modal('hide');
        util.reboot();
    });
}

export function updateLockState(module_init) {
    $('#sidebar-mqtt').prop('hidden', !module_init.mqtt);
}

export function getTranslation(lang: string) {
    return {
        "de": {
            "mqtt": {
                "status": {
                    "connection": "MQTT-Verbindung"
                },
                "navbar": {
                    "mqtt": "MQTT"
                },
                "content": {
                    "mqtt": "MQTT-Einstellungen",
                    "enable_mqtt_desc": "Hierdurch kann das Gerät über den konfigurierten MQTT-Broker kontrolliert werden. Siehe <b>TODO: MQTT API documentation link</b> für Details.",
                    "enable_mqtt": "MQTT aktiviert",
                    "broker_host": "Broker-Hostname oder -IP",
                    "port": "Broker-Port (typischerweise 1883)",
                    "username": "Broker-Username (optional)",
                    "password": "Broker-Passwort (optional)",
                    "topic_prefix": "Topic-Präfix",
                    "client_name": "Name des Clients",
                    "save": "Speichern",
                    "reboot_title": "Neu starten um Konfiguration anzuwenden",
                    "reboot_content": "Die geänderten MQTT-Einstellungen werden nur nach einem Neustart angewendet. Jetzt neu starten?",
                    "abort": "Abbrechen",
                    "reboot": "Neu starten",
                    "show_password": "Anzeigen"
                },
                "script": {
                    "save_failed": "Speichern der MQTT-Einstellungen fehlgeschlagen.",
                    "to": "zu"
                }
            }
        },
        "en": {
            "mqtt": {
                "status": {
                    "connection": "MQTT Connection"
                },
                "navbar": {
                    "mqtt": "MQTT"
                },
                "content": {
                    "mqtt": "MQTT Settings",
                    "enable_mqtt_desc": "This allows controlling the device over the configured MQTT broker. See <b>TODO: MQTT API documentation link</b> for details.",
                    "enable_mqtt": "Enable MQTT",
                    "broker_host": "Broker hostname or IP",
                    "port": "Broker port (typically 1883)",
                    "username": "Broker username (optional)",
                    "password": "Broker password (optional)",
                    "topic_prefix": "Topic prefix",
                    "client_name": "Client name",
                    "save": "Save",
                    "reboot_title": "Reboot to apply configuration",
                    "reboot_content": "The changed MQTT configuration will only be applied after rebooting. Reboot now?",
                    "abort": "Abort",
                    "reboot": "Reboot",
                    "show_password": "Show"
                },
                "script": {
                    "save_failed": "Failed to save the MQTT configuration.",
                    "to": "to"
                }
            }
        }
    }[lang];
}
