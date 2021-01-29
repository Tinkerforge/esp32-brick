import $ from "jquery";

import * as util from "../util";

import bsCustomFileInput from "bs-custom-file-input";

interface Version {
    firmware: string,
    spiffs: string
}

function update_version(version: Version) {
    $('#current_firmware').val(version.firmware);
    $('#current_spiffs').val(version.spiffs);
}

function upload(e: JQuery.SubmitEvent, type: string) {
    e.preventDefault();

    let file_select = <HTMLInputElement>$(`#${type}_file_select`)[0];
    let progress = $(`#${type}-progress`);
    let progress_bar = $(`#${type}-progress-bar`);

    progress.prop("hidden", false);
    let data = new FormData();
    data.append(type, file_select.files[0]);

    $.ajax({
        url: `/flash_${type}`,
        type: 'POST',
        data: data,
        contentType: false,
        processData: false,
        xhr: function () {
            let xhr = new window.XMLHttpRequest();
            xhr.upload.addEventListener('progress', function (evt) {
                if (evt.lengthComputable) {
                    let per = evt.loaded / evt.total;
                    progress_bar.prop('style', "width: " + (per * 100) + "%");
                    progress_bar.prop('aria-valuenow', (per * 100));
                }
            }, false);
            return xhr;
        },
        success: () => {
            progress.prop("hidden", true);
                util.show_alert("alert-success", __("firmware_update.script.flash_success"), __("firmware_update.script.flash_reboot"));
        },
        error: (xhr, status, error) => {
            progress.prop("hidden", true);
            if (__(xhr.responseText) == null)
                util.show_alert("alert-danger", __("firmware_update.script.flash_fail"), error + ": " + xhr.responseText);
            else
                util.show_alert("alert-danger", __("firmware_update.script.flash_fail"), __(xhr.responseText));
        }
    });
}

function factory_reset_modal() {
    $('#factory_reset_modal').modal('show');
}

function factory_reset() {
    $.ajax({
        url: `/factory_reset`,
        type: 'POST',
        contentType: "application/json; charset=utf-8",
        data: JSON.stringify({"do_i_know_what_i_am_doing": true}),
        success: () => {
            $('#factory_reset_modal').modal('hide');
            util.show_alert("alert-success", __("firmware_update.script.factory_reset_init"), __("firmware_update.script.factory_reset_reboot"));
        },
        error: (xhr, status, error) => {
            $('#factory_reset_modal').modal('hide');
            util.show_alert("alert-danger", __("firmware_update.script.factory_reset_error"), error + ": " + xhr.responseText);
        }
    });
}

export function init() {
    //Firmware upload
    $('#upload_firmware_form').on("submit", function (e) {
        upload(e, "firmware");
    });

    $('#firmware_file_select').on("change", () => $("#update_firmware_button").prop("disabled", false));

    $('#spiffs_factory_reset').on("click", factory_reset_modal);
    $('#factory_reset_confirm').on("click", factory_reset);

    $('#download_debug_report').on("click", () => $('#download_debug_report').attr("download", "debug-report-" + (new Date()).toISOString().replace(/:/gi, "-").replace(/\./gi, "-") + ".json"));

    bsCustomFileInput.init();
}

export function addEventListeners(source: EventSource) {
    source.addEventListener('version', function (e: util.SSE) {
        update_version(<Version>(JSON.parse(e.data)));
    }, false);
}


export function updateLockState(module_init) {
    $('#sidebar-flash').prop('hidden', !module_init.firmware_update);
}

export function getTranslation(lang: string) {
    return {
        "de": {
            "firmware_update": {
                "status": {

                },
                "navbar": {
                    "system": "System"
                },
                "content": {
                    "system": "System",
                    "current_firmware": "Firmwareversion",
                    "firmware_update": "Firmware-Aktualisierung",
                    "browse": "Durchsuchen",
                    "select_file": "Firmware-Datei",
                    "update": "Hochladen",
                    "uploading": "Firmware wird hochgeladen...",
                    "current_spiffs": "Konfigurationsversion",
                    "factory_reset_desc": "Setzt alle Einstellungen auf den Auslieferungs&shy;zustand zurück",
                    "factory_reset": "Zurücksetzen",
                    "factory_reset_modal_text": "Hiermit werden alle Einstellungen auf den Auslieferungszustand zurückgesetzt. <b>Diese Aktion kann nicht rückgängig gemacht werden!</b> Die Wallbox öffnet dann wieder einen eigenen WLAN-Access-Point mit dem Netzwerknamen (SSID) und der Passphrase aus dem Handbuch. Fortfahren?",
                    "abort_reset": "Abbrechen",
                    "confirm_reset": "Zurücksetzen auf Auslieferungszustand",
                    "debug_report": "Debug-Report",
                    "debug_report_desc": "Ein kompletter Report aller Statusinformationen des Geräts außer Passwörtern"
                },
                "script": {
                    "flash_success": "Erfolgreich aktualisiert",
                    "flash_reboot": "Starte neu... Bitte das Web-Interface in ungefähr 30 Sekunden neuladen.",
                    "flash_fail": "Aktualisierung fehlgeschlagen",
                    "factory_reset_init": "Beginne Zurücksetzen auf Auslieferungszustand",
                    "factory_reset_reboot": "Formatiere Konfigurationspartition und starte neu... Dies dauert ungefähr eine Minute.",
                    "factory_reset_error": "Zurücksetzen auf Auslieferungszustand fehlgeschlagen",
                    "vehicle_connected": "Es kann keine Aktualisierung vorgenommen werden, während ein Fahrzeug verbunden ist."
                }
            }
        },
        "en": {
            "firmware_update": {
                "status": {

                },
                "navbar": {
                    "system": "System"
                },
                "content": {
                    "system": "System",
                    "current_firmware": "Firmware version",
                    "firmware_update": "Firmware Update",
                    "browse": "Browse",
                    "select_file": "Firmware file",
                    "update": "Upload",
                    "uploading": "Uploading Firmware...",
                    "current_spiffs": "Configuration version",
                    "factory_reset_desc": "Restores all settings to factory defaults",
                    "factory_reset": "Factory Reset",
                    "factory_reset_modal_text": "A factory reset will reset all settings to their default values. <b>This action cannot be undone!</b> The wallbox will re-open the WiFi access point with the default SSID and passphrase. Are you sure?",
                    "abort_reset": "Abort",
                    "confirm_reset": "Restore",
                    "debug_report": "Debug report",
                    "debug_report_desc": "A complete report of all status information of this device except passwords"
                },
                "script": {
                    "flash_success": "Successfully updated",
                    "flash_reboot": "Rebooting... Please reload the web interface in about 30 seconds.",
                    "flash_fail": "Failed to update",
                    "factory_reset_init": "Initiated factory reset",
                    "factory_reset_reboot": "Formatting configuration partition and rebooting... This takes about one minute.",
                    "factory_reset_error": "Initiating factory reset failed",
                    "vehicle_connected": "Can't update the firmware while a vehicle is connected."
                }
            }
        }
    }[lang];
}
