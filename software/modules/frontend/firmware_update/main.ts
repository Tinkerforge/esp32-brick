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
        statusCode: {
            200: function () {
                progress.prop("hidden", true);
                util.show_alert("alert-success", __("firmware_update.script.flash_success"), __("firmware_update.script.flash_reboot"));
            },
            400: function (result) {
                progress.prop("hidden", true);
                util.show_alert("alert-danger", __("firmware_update.script.flash_fail"), result.responseText);
            }
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
        error: (_x, _y, error) => {
            $('#factory_reset_modal').modal('hide');
            util.show_alert("alert-danger", __("firmware_update.script.factory_reset_error"), error);
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
                    "current_firmware": "Installierte Firmware",
                    "firmware_update": "Firmware-Aktualisierung",
                    "browse": "Durchsuchen",
                    "select_file": "Firmware-Datei auswählen...",
                    "update": "Hochladen",
                    "uploading": "Firmware wird hochgeladen...",
                    "current_spiffs": "Konfigurationsversion",
                    "factory_reset_desc": "Setzt alle Einstellungen auf den Werkszustand zurück",
                    "factory_reset": "Zurücksetzen auf Werkseinstellungen",
                    "factory_reset_modal_text": "Hiermit werden alle Einstellungen auf den Werkszustand zurückgesetzt. <b>Diese Aktion kann nicht rückgängig gemacht werden!</b> Die Wallbox öffnet dann wieder einen eigenen WLAN-Access-Point mit dem Netzwerk-Namen (SSID) und der Passphrase aus dem Handbuch. Fortfahren?",
                    "abort_reset": "Abbrechen",
                    "confirm_reset": "Zurücksetzen auf Werkseinstellungen",
                    "debug_report": "Debug-Report herunterladen",
                    "debug_report_desc": "Ein kompletter Report aller Statusinformationen des Geräts außer Passwörtern"
                },
                "script": {
                    "flash_success": "Erfolgreich aktualisiert",
                    "flash_reboot": "Starte neu... Bitte das Web-Interface in ungefähr 30 Sekunden neuladen.",
                    "flash_fail": "Aktualisierung fehlgeschlagen",
                    "factory_reset_init": "Beginne Zurücksetzen auf Werkseinstellungen",
                    "factory_reset_reboot": "Formatiere Konfigurationspartition und starte neu... Dies dauert ungefähr eine Minute.",
                    "factory_reset_error": "Zurücksetzen auf Werkseinstellungen fehlgeschlagen",
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
                    "current_firmware": "Installed firmware",
                    "firmware_update": "Firmware Update",
                    "browse": "Browse",
                    "select_file": "Select firmware file...",
                    "update": "Upload",
                    "uploading": "Uploading Firmware...",
                    "current_spiffs": "Configuration version",
                    "factory_reset_desc": "Reset all settings to factory default",
                    "factory_reset": "Factory Reset",
                    "factory_reset_modal_text": "A factory reset will reset all settings to their default values. <b>This action cannot be undone!</b> The wallbox will re-open the WiFi access point with the default SSID and passphrase. Are you sure?",
                    "abort_reset": "Abort",
                    "confirm_reset": "Reset to factory defaults",
                    "debug_report": "Download debug report",
                    "debug_report_desc": "A complete report of all status information of this device except passwords"
                },
                "script": {
                    "flash_success": "Successfully updated",
                    "flash_reboot": "Rebooting... Please reload the web interface in about 30 seconds.",
                    "flash_fail": "Failed to update",
                    "factory_reset_init": "Initiated factory reset",
                    "factory_reset_reboot": "Formatting configuration partition and rebooting... This takes about one minute.",
                    "factory_reset_error": "Initiating factory reset failed",
                }
            }
        }
    }[lang];
}
