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

import $ from "jquery";

import * as util from "../util";

declare function __(s: string): string;

interface AuthenticationConfig {
    enable_auth: boolean,
    username: string,
    password: string
}

function update_authentication_config(config: AuthenticationConfig) {
    $('#authentication_enable').prop("checked", config.enable_auth);
    $('#authentication_username').val(config.username);
    // omit password
}

function save_authentication_config() {
    let payload: AuthenticationConfig = {
        enable_auth: $('#authentication_enable').is(':checked'),
        username: $('#authentication_username').val().toString(),
        password: util.passwordUpdate($('#authentication_password').val().toString()),
    };

    $.ajax({
        url: '/authentication/config_update',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(payload),
        success: () => $('#authentication_reboot').modal('show'),
        error: (xhr, status, error) => util.show_alert("alert-danger", __("authentication.script.save_failed"), error + ": " + xhr.responseText)
    });
}


export function init() {
    let button = <HTMLButtonElement>document.getElementById("authentication_show_password");
    button.addEventListener("change", util.toggle_password_fn("#authentication_password"));

    let form = <HTMLFormElement>$('#authentication_config_form')[0];
    form.addEventListener('submit', function (event: Event) {
        form.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (form.checkValidity() === false) {
            return;
        }

        save_authentication_config();
    }, false);

    (<HTMLButtonElement>document.getElementById("authentication_reboot_button")).addEventListener("click", () => {
        $('#authentication_reboot').modal('hide');
        util.reboot();
    });
}

export function addEventListeners(source: EventSource) {
    source.addEventListener('authentication/config', function (e: util.SSE) {
        update_authentication_config(<AuthenticationConfig>(JSON.parse(e.data)));
    }, false);
}

export function updateLockState(module_init: any) {
    $('#sidebar-authentication').prop('hidden', !module_init.authentication);
    $('#sidebar-system').prop('hidden',  $('#sidebar-system').prop('hidden') && !module_init.authentication);
}

export function getTranslation(lang: string) {
    const translations: {[index: string]:any} = {
        "de": {
            "authentication": {
                "status": {

                },
                "navbar": {
                    "authentication": "Zugangsdaten"
                },
                "content": {
                    "authentication": "Zugangsdaten",
                    "enable_authentication": "Anmeldung aktiviert",
                    "enable_authentication_desc": "Wenn aktiviert, muss beim Aufrufen des Webinterfaces oder bei Verwendung der HTTP-API eine Anmeldung mit den konfigurierten Zugangsdaten durchgeführt werden.",
                    "username": "Benutzername",
                    "password": "Passwort",
                    "unchanged": "Unverändert",
                    "show_password": "Anzeigen",
                    "save": "Speichern",
                    "reboot_title": "Neu starten um Konfiguration anzuwenden",
                    "reboot_content": "Die geänderten Zugangsdaten werden nur nach einem Neustart angewendet. Jetzt neu starten?",
                    "abort": "Abbrechen",
                    "reboot": "Neu starten"
                },
                "script": {
                    "save_failed": "Speichern der Zugangsdaten fehlgeschlagen.",
                }
            }
        },
        "en": {
            "authentication": {
                "status": {

                },
                "navbar": {
                    "authentication": "Credentials"
                },
                "content": {
                    "authentication": "Credentials",
                    "enable_authentication": "Enable authentication",
                    "enable_authentication_desc": "If activated, the configured credentials must be entered to open the web interface or use the HTTP API.",
                    "username": "Username",
                    "password": "Password",
                    "unchanged": "Unchanged",
                    "show_password": "Show",
                    "save": "Save",
                    "reboot_title": "Reboot to apply configuration",
                    "reboot_content": "The changed credentials will only be applied after rebooting. Reboot now?",
                    "abort": "Abort",
                    "reboot": "Reboot",
                },
                "script": {
                    "save_failed": "Failed to save the credentials.",
                }
            }
        }
    };
    return translations[lang];
}
