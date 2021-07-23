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

function load_event_log() {
    $.get("/event_log")
               .done((result) => {$('#event_log_content').html(result)})
               .fail((xhr, status, error) => util.add_alert("event_log_load_failed", "alert-danger", __("event_log.script.load_event_report_error"), error + ": " + xhr.responseText))
}

let update_event_log_interval: number = null;

export function init() {
    $('#sidebar-event_log').on('shown.bs.tab', function (e) {
        load_event_log();
        if (update_event_log_interval == null) {
            update_event_log_interval = window.setInterval(load_event_log, 10000);
        }
    });

    $('#sidebar-event_log').on('hidden.bs.tab', function (e) {
        if (update_event_log_interval != null) {
            clearInterval(update_event_log_interval);
            update_event_log_interval = null;
        }
    });

    $('#download_debug_report').on("click", () => $('#download_debug_report').attr("download", "debug-report-" + (new Date()).toISOString().replace(/:/gi, "-").replace(/\./gi, "-") + ".json"));

    $('#load_event_log').on("click", load_event_log);

    $('#save_event_log').on("click", () => $('#save_event_log').attr("download", "event-log-" + (new Date()).toISOString().replace(/:/gi, "-").replace(/\./gi, "-") + ".txt"));
}

export function addEventListeners(source: EventSource) {}

export function updateLockState(module_init: any) {
    $('#sidebar-event-log').prop('hidden', !module_init.event_log);
    $('#sidebar-system').prop('hidden',  $('#sidebar-system').prop('hidden') && !module_init.event_log);
}

export function getTranslation(lang: string) {
    const translations: {[index: string]:any} = {
        "de": {
            "event_log": {
                "status": {

                },
                "navbar": {
                    "event_log": "Ereignis-Log"
                },
                "content": {
                    "event_log": "Ereignis-Log",
                    "debug_report": "Debug-Report",
                    "debug_report_desc": "Ein kompletter Report aller Statusinformationen des Geräts außer Passwörtern",
                    "event_log_placeholder": "Lade Ereignis-Log...",
                    "save_event_log": "Log speichern",
                    "save_event_log_desc": "Speichert das aktuelle Ereignis-Log",
                },
                "script": {
                    "load_event_report_error": "Konnte Ereignis-Log nicht laden",
                }
            }
        },
        "en": {
            "event_log": {
                "status": {

                },
                "navbar": {
                    "event_log": "Event Log"
                },
                "content": {
                    "event_log": "Event Log",
                    "debug_report": "Debug report",
                    "debug_report_desc": "A complete report of all status information of this device except passwords",
                    "event_log_placeholder": "Loading event log...",
                    "save_event_log": "Save log",
                    "save_event_log_desc": "Saves the current event log",
                },
                "script": {
                    "load_event_report_error": "Failed to load event log",
                }
            }
        }
    };
    return translations[lang];
}
