import $ from "jquery";

import "bootstrap";

import feather = require("feather-icons");

import * as util from "../util";

interface WifiInfo {
    ssid: string,
    bssid: string,
    rssi: number,
    channel: number,
    encryption: number
}

interface GetWifiResult {
    result: WifiInfo[]
}

function scan_wifi() {
    $.get("/scan_wifi").done(function () {
        $("#wifi_config_scan_spinner").prop('hidden', false);
        $("#wifi_scan_results").prop('hidden', true);
        $("#wifi_config_carousel").carousel(1);

        setTimeout(function () {
            $.get("/get_wifis").done(function (data: GetWifiResult) {
                $("#wifi_config_scan_spinner").prop('hidden', true);
                $("#wifi_scan_results").prop('hidden', false);
                $("#wifi_scan_title").html(`<h4>${__("wifi.script.select_ap")}</h4>`);

                if (data.result.length == 0) {
                    $("#wifi_scan_results").html(__("wifi.script.no_ap_found"));
                    return;
                }
                let result = `<div class="table-responsive col-lg-6"><table id="wifi_config_found_aps" class="table table-hover">
                <thead>
                    <tr>
                    <th scope="col">${__("wifi.script.ssid")}</th>
                    <th scope="col">${__("wifi.script.rssi")}</th>
                    <th scope="col">${__("wifi.script.password")}</th>
                    <th scope="col"></th>
                    </tr>
                </thead>
                <tbody>`;

                $.each(data.result, (i, v: WifiInfo) => {
                    let line = "<tr>";
                    line += "<td>" + v.ssid + "</td>";
                    line += "<td>" + v.rssi + "</td>";
                    if (v.encryption == 0)
                        line += "<td><span data-feather='unlock'></td>";
                    else
                        line += "<td><span data-feather='lock'></td>";
                    line += `<td><button id="wifi_scan_result_${i}" type="button" class="btn btn-primary">${__("wifi.script.connect")}</button></td>`
                    line += "</tr>"

                    result += line;
                });
                result += "</tbody></table></div>";
                $("#wifi_scan_results").html(result);

                $.each(data.result, (i, v: WifiInfo) => {
                    let button = document.getElementById(`wifi_scan_result_${i}`);
                    button.addEventListener("click", () => connect_to_ap(v.ssid, v.bssid, v.encryption));
                });

                feather.replace();
            });
        }, 5000);
    });
}

interface WifiSTAConfig {
    ssid: string,
    bssid: number[],
    bssid_lock: boolean,
    passphrase: string,
    hostname: string,
    ip: number[],
    gateway: number[],
    subnet: number[],
    dns: number[],
    dns2: number[],
}

function update_wifi_sta_config(config: WifiSTAConfig) {
    if (config.ssid == "")
        $('#wifi_sta_connection_name').html(`<span class="form-label">${__("wifi.script.wifi_connection")}</span>`);
    else
        $('#wifi_sta_connection_name').html(`<span class="form-label">${__("wifi.script.wifi_connection_pre")} ${config.ssid}</span>`);

    if (config.ssid != "") {
        $('#wifi_ap_configuration_state').val(config.ssid);
        //$('#wifi_save_enable_sta').removeProp("disabled");
        //$('#wifi_save_ap_fallback_only').removeProp("disabled");
    }
    else {
        $('#wifi_ap_configuration_state').val(__("wifi.script.no_ap_configured"));
        //$('#wifi_save_enable_sta').prop("disabled", true);
        //$('#wifi_save_ap_fallback_only').prop("disabled", true);
    }
}

interface WifiState {
    connection_state: number,
    ap_state: number
}

function update_wifi_state(state: WifiState) {
    let sta_button_dict: { [idx: number]: string } = {
        0: "#wifi_sta_not_configured",
        1: "#wifi_sta_not_connected",
        2: "#wifi_sta_connecting",
        3: "#wifi_sta_connected",
    };

    let sta_outline_dict: { [idx: number]: string } = {
        0: "btn-outline-primary",
        1: "btn-outline-danger",
        2: "btn-outline-warning",
        3: "btn-outline-success",
    };

    let sta_non_outline_dict: { [idx: number]: string } = {
        0: "btn-primary",
        1: "btn-danger",
        2: "btn-warning",
        3: "btn-success",
    };

    for (let i = 0; i < 4; ++i) {
        $(sta_button_dict[i]).removeClass(sta_non_outline_dict[i]);
        $(sta_button_dict[i]).addClass(sta_outline_dict[i]);
    }

    let button_to_highlight = state.connection_state + 1;
    $(sta_button_dict[button_to_highlight]).removeClass(sta_outline_dict[button_to_highlight]);
    $(sta_button_dict[button_to_highlight]).addClass(sta_non_outline_dict[button_to_highlight]);


    let ap_button_dict: { [idx: number]: string } = {
        0: "#wifi_ap_disabled",
        1: "#wifi_ap_enabled",
        2: "#wifi_ap_fallback_inactive",
        3: "#wifi_ap_fallback_active",
    };

    let ap_outline_dict: { [idx: number]: string } = {
        0: "btn-outline-primary",
        1: "btn-outline-success",
        2: "btn-outline-success",
        3: "btn-outline-danger",
    };

    let ap_non_outline_dict: { [idx: number]: string } = {
        0: "btn-primary",
        1: "btn-success",
        2: "btn-success",
        3: "btn-danger",
    };

    for (let i = 0; i < 4; ++i) {
        $(ap_button_dict[i]).removeClass(ap_non_outline_dict[i]);
        $(ap_button_dict[i]).addClass(ap_outline_dict[i]);
    }

    button_to_highlight = state.ap_state
    $(ap_button_dict[button_to_highlight]).removeClass(ap_outline_dict[button_to_highlight]);
    $(ap_button_dict[button_to_highlight]).addClass(ap_non_outline_dict[button_to_highlight]);
}

interface WifiConfig {
    enable_soft_ap: boolean,
    enable_sta: boolean,
    ap_fallback_only: boolean,
}

function update_wifi_config(config: WifiConfig) {
    $('#wifi_save_enable_sta').prop("checked", config.enable_sta);
    $('#wifi_save_enable_ap').prop("checked", config.enable_soft_ap);
    $('#wifi_save_ap_fallback_only').prop("checked", config.ap_fallback_only);
}

function wifi_cfg_toggle_static_ip_collapse(value: string) {
    if (value == "hide") {
        $('#wifi_cfg_static_ip_cfg').collapse('hide');
        $('#wifi_cfg_static_ip_ip').prop('required', false);
        $('#wifi_cfg_static_ip_subnet').prop('required', false);
        $('#wifi_cfg_static_ip_gateway').prop('required', false);
    }
    else if (value == "show") {
        $('#wifi_cfg_static_ip_cfg').collapse('show');
        $('#wifi_cfg_static_ip_ip').prop('required', true);
        $('#wifi_cfg_static_ip_subnet').prop('required', true);
        $('#wifi_cfg_static_ip_gateway').prop('required', true);
    }
    $('#wifi_configuration').modal('handleUpdate');
}

function toggle_show_passphrase() {
    let input = <HTMLInputElement>$('#wifi_cfg_passphrase')[0];
    if (input.type == 'password')
        input.type = 'text';
    else
        input.type = 'password';
}

function connect_to_ap(ssid: string, bssid: string, encryption: number) {
    let form_passphrase = ""
    let form_bssid_lock = "";
    let form_ssid = `<span id="wifi_cfg_ssid"></span>${__("wifi.script.hidden_ap")} (<span id="wifi_cfg_bssid">${bssid}</span>)`;

    if (encryption != 0) {
        form_passphrase = `<div class="form-group row no-gutters" id="wifi_cfg_passphrase_form">
                <label for="wifi_cfg_passphrase" class="col-sm-3 col-form-label">${__("wifi.script.passphrase")}</label>
                <div class="col-sm-9">
                    <div class="input-group">
                        <input type="password" id="wifi_cfg_passphrase" class="form-control" value="" required minlength="8" maxlength="63"/>
                        <div class="input-group-append">
                            <div class="input-group-text custom-control custom-switch" style="padding-left: 2.5rem;">
                                <input id="wifi_cfg_show_passphrase" type="checkbox" class="custom-control-input" aria-label="Show passphrase" value="test">
                                <label class="custom-control-label" for="wifi_cfg_show_passphrase">${__("wifi.script.show")}</label>
                            </div>
                        </div>
                        <div class="invalid-feedback">
                            ${__("wifi.script.passphrase_requirements")}
                        </div>
                    </div>
                </div>
            </div>`;
    }

    if (ssid != "") {
        form_bssid_lock = `<div class="form-group row no-gutters">
                <label for="wifi_cfg_bssid_lock" class="col-sm-3 col-form-label">${__("wifi.script.bssid_lock")}</label>
                <div class="col-sm-9">
                    <div class="custom-control custom-switch">
                        <input type="checkbox" class="custom-control-input" id="wifi_cfg_bssid_lock">
                        <label class="custom-control-label" for="wifi_cfg_bssid_lock">
                            ${__("wifi.script.bssid_lock_desc_pre")} ${bssid}${__("wifi.script.bssid_lock_desc_post")}
                        </label>
                    </div>
                </div>
            </div>`;
        form_ssid = `<span id="wifi_cfg_ssid">${ssid}</span> (<span id="wifi_cfg_bssid">${bssid}</span>)`;
    }

    let form_content = `
    <div class="d-flex justify-content-between flex-wrap flex-md-nowrap align-items-center pb-2 mb-3">
        <h4>${__("wifi.script.connect_to")} ${form_ssid}</h4>
    </div>
    <div>
        <form id="wifi_config_form" class="needs-validation" novalidate>
        <!--
            Use custom validation instead of the browser default, that is broken on Chrome
            See: https://getbootstrap.com/docs/4.5/components/forms/#validation
        -->

            ${form_passphrase}
            <div class="form-group row no-gutters">
                <div id="blah" class="col-sm-3">
                    <label for="wifi_cfg_ip_configuration" class="col-form-label">${__("wifi.script.ip_configuration")}</label>
                </div>

                <div class="col-sm-9">
                    <select id="wifi_cfg_ip_configuration" class="custom-select">
                        <!-- TODO better toggle behaviour-->
                        <option value="hide" selected>${__("wifi.script.dhcp")}</option>
                        <option value="show">${__("wifi.script.static")}</option>
                    </select>
                </div>
            </div>
            <div class="collapse form-group row no-gutters justify-content-end"
                id="wifi_cfg_static_ip_cfg">
                <div class="card col-sm-9">
                    <div class="card-body">
                        <h5 class="card-title">${__("wifi.script.static_title")}</h5>
                        <div class="form-group row justify-content-end">
                            <label for="wifi_cfg_static_ip_ip"
                                class="col-sm-3 col-form-label">${__("wifi.script.static_ip")}</label>
                            <div class="col-sm-9">
                                <input id="wifi_cfg_static_ip_ip" class="form-control" type="text"
                                    minlength="7" maxlength="15" size="15"
                                    pattern="^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$">
                                <div class="invalid-feedback">
                                    ${__("wifi.script.static_ip_invalid")}
                                </div>
                            </div>
                        </div>

                        <div class="form-group row justify-content-end">
                            <label for="wifi_cfg_static_ip_subnet"
                                class="col-sm-3 col-form-label">${__("wifi.script.subnet_mask")}</label>
                            <div class="col-sm-9">
                                <input id="wifi_cfg_static_ip_subnet" class="form-control" type="text"
                                    minlength="7" maxlength="15" size="15"
                                    pattern="^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$">
                                <div class="invalid-feedback">
                                    ${__("wifi.script.subnet_mask_invalid")}
                                </div>
                            </div>
                        </div>

                        <div class="form-group row justify-content-end">
                            <label for="wifi_cfg_static_ip_gateway"
                                class="col-sm-3 col-form-label">${__("wifi.script.gateway")}</label>
                            <div class="col-sm-9">
                                <input id="wifi_cfg_static_ip_gateway" class="form-control" type="text"
                                    minlength="7" maxlength="15" size="15"
                                    pattern="^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$">
                                <div class="invalid-feedback">
                                    ${__("wifi.script.gateway_invalid")}
                                </div>
                            </div>
                        </div>

                        <div class="form-group row justify-content-end">
                            <label for="wifi_cfg_static_ip_dns" class="col-sm-3 col-form-label">${__("wifi.script.dns")}</label>
                            <div class="col-sm-9">
                                <input id="wifi_cfg_static_ip_dns" class="form-control" type="text"
                                    minlength="7" maxlength="15" size="15"
                                    pattern="^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$">
                                <div class="invalid-feedback">
                                    ${__("wifi.script.dns_invalid")}
                                </div>
                            </div>
                        </div>

                        <div class="form-group row justify-content-end">
                            <label for="wifi_cfg_static_ip_dns2" class="col-sm-3 col-form-label">${__("wifi.script.dns2")}</label>
                            <div class="col-sm-9">
                                <input id="wifi_cfg_static_ip_dns2" class="form-control" type="text"
                                    minlength="7" maxlength="15" size="15"
                                    pattern="^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$">
                                <div class="invalid-feedback">
                                    ${__("wifi.script.dns2_invalid")}
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
            ${form_bssid_lock}
        </form>
    </div>
    <div class="text-right">
        <button id="wifi_config_submit_button" type="submit" form="wifi_config_form" class="btn btn-primary">${__("wifi.script.next")}</button>
    </div>`;

    $('#wifi_config_content').html(form_content);
    if (encryption != 0) {
        let button = <HTMLButtonElement>document.getElementById("wifi_cfg_show_passphrase");
        button.addEventListener("change", toggle_show_passphrase);
    }

    let input = <HTMLInputElement>document.getElementById("wifi_cfg_ip_configuration");
    input.addEventListener("change", () => wifi_cfg_toggle_static_ip_collapse(input.value));

    $('#wifi_config_carousel').carousel(2);

    // Use bootstrap form validation
    let form = <HTMLFormElement>$('#wifi_config_form')[0];
    form.addEventListener('submit', function (event: Event) {
        form.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (form.checkValidity() === false) {
            return;
        }
        $('#wifi_config_connect_to_ap_switch').html(`${__("wifi.script.sta_on_boot_pre")} ${$('#wifi_cfg_ssid').text()} ${__("wifi.script.sta_on_boot_post")}`);

        $('#wifi_config_ap_fallback_switch').html(`${__("wifi.script.ap_as_fallback_pre")} ${$('#wifi_cfg_ssid').text()} ${__("wifi.script.ap_as_fallback_post")}`);
        $('#wifi_config_carousel').carousel("next");
    }, false);
}

function parse_ip(ip_str: string) {
    let splt = ip_str.split('.');
    if (splt.length != 4)
        return [0, 0, 0, 0];

    let result: number[] = [];

    for (let i = 0; i < 4; ++i)
        result.push(parseInt(splt[i], 10));
    return result;
}

function save_wifi_sta_config(continuation = function () { }) {
    let dhcp = $('#wifi_cfg_ip_configuration').val() != "show";

    let payload: WifiSTAConfig = {
        hostname: "wallbox",
        ssid: $('#wifi_cfg_ssid').text(),
        bssid: $('#wifi_cfg_bssid').text().split(':').map(x => parseInt(x, 16)),
        bssid_lock: $('#wifi_cfg_bssid_lock').is(':checked'),
        passphrase: $('#wifi_cfg_passphrase').length > 0 ? $('#wifi_cfg_passphrase').val().toString() : "",
        ip: dhcp ? [0, 0, 0, 0] : parse_ip($('#wifi_cfg_static_ip_ip').val().toString()),
        subnet: dhcp ? [0, 0, 0, 0] : parse_ip($('#wifi_cfg_static_ip_subnet').val().toString()),
        gateway: dhcp ? [0, 0, 0, 0] : parse_ip($('#wifi_cfg_static_ip_gateway').val().toString()),
        dns: dhcp ? [0, 0, 0, 0] : parse_ip($('#wifi_cfg_static_ip_dns').val().toString()),
        dns2: dhcp ? [0, 0, 0, 0] : parse_ip($('#wifi_cfg_static_ip_dns2').val().toString())
    };

    $.ajax({
        url: '/wifi_sta_config',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(payload),
        success: continuation,
        error: (_x, _y, error) => util.show_alert("alert-danger", __("wifi.script.sta_config_failed"), error)
    });
}

function save_all_wifi_config() {
    save_wifi_sta_config(save_wifi_config);
}

function save_wifi_config() {
    let payload: WifiConfig = {
        enable_sta: $('#wifi_save_enable_sta').is(':checked'),
        enable_soft_ap: $('#wifi_save_enable_ap').is(':checked'),
        ap_fallback_only: $('#wifi_save_ap_fallback_only').is(':checked')
    }

    $.ajax({
        url: '/wifi_config',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(payload),
        success: () => $('#wifi_reboot').modal('show'),
        error: (_x, _y, error) => util.show_alert("alert-danger", __("wifi.script.config_failed"), error)
    });

    return;
}

function wifi_save_reboot() {
    $('#wifi_reboot').modal('hide');
    $("#wifi_config_carousel").carousel(0);
    util.reboot();
}

$('#wifi_config_carousel').on('slid.bs.carousel', function (e) {
    $('#wifi_carousel_back_button').prop("hidden", e.to == 0);
    $('#wifi_carousel_abort_button').prop("hidden", e.to == 0);
});

export function addEventListeners(source: EventSource) {
    source.addEventListener('wifi_config', function (e: util.SSE) {
        update_wifi_config(<WifiConfig>(JSON.parse(e.data)));
    }, false);

    source.addEventListener('wifi_state', function (e: util.SSE) {
        update_wifi_state(<WifiState>(JSON.parse(e.data)));
    }, false);

    source.addEventListener('wifi_sta_config', function (e: util.SSE) {
        update_wifi_sta_config(<WifiSTAConfig>(JSON.parse(e.data)));
    }, false);
}

export function init() {
    let input2 = <HTMLInputElement>document.getElementById("wifi_save_enable_ap");
    input2.addEventListener("change", () => $("#wifi_save_ap_fallback_only").prop("disabled", !input2.checked));

    (<HTMLButtonElement>document.getElementById("wifi_carousel_abort_button")).addEventListener("click", () => $("#wifi_config_carousel").carousel(0));

    (<HTMLButtonElement>document.getElementById("wifi_carousel_back_button")).addEventListener("click", () => $("#wifi_config_carousel").carousel("prev"));

    (<HTMLButtonElement>document.getElementById("scan_wifi_button")).addEventListener("click", scan_wifi);

    (<HTMLButtonElement>document.getElementById("wifi_config_ap_fallback_save")).addEventListener("click", save_all_wifi_config);

    (<HTMLButtonElement>document.getElementById("wifi_reboot_button")).addEventListener("click", wifi_save_reboot);
}


export function updateLockState(module_init) {
    $('#sidebar-wifi').prop('hidden', !module_init.wifi);
}

export function getTranslation(lang: string) {
    return {
        "de": {
            "wifi": {
                "status": {
                    "wifi_connection": "WLAN-Verbindung",
                    "not_configured": "Nicht konfiguriert",
                    "not_connected": "Nicht verbunden",
                    "connecting": "Verbinde",
                    "connected": "Verbunden",
                    "wifi_ap": "WLAN-Access-Point",
                    "deactivated": "Deaktiviert",
                    "activated": "Aktiviert",
                    "fallback_inactive": "Fallback inaktiv",
                    "fallback_active": "Fallback aktiv"
                },
                "navbar": {
                    "wifi": "WLAN"
                },
                "content": {
                    "wifi_settings": "WLAN-Einstellungen",
                    "cancel": "Abbrechen",
                    "back": "Zurück",
                    "client_configuration": "Verbindungseinstellungen",
                    "configured_ap": "Konfigurierter Access Point",
                    "configure_connection": "Verbindung zu anderem Access Point konfigurieren",
                    "connect_to_wifi": "Zu WLAN verbinden",
                    "scanning_for_aps": "Scanne verfügbare Access Points...",
                    "scanning": "Scanne...",
                    "configure_ap_mode": "WLAN-Starteinstellungen",
                    "sta_on_boot": "Verbinde beim Start zum Access Point",
                    "ap_on_boot": "Öffne beim Start eigenen Accesspoint",
                    "ap_as_fallback": "Öffne eigenen Accesspoint nur, falls die Verbindung zum Access Point fehlschlägt",
                    "finish": "Abschließen",
                    "reboot_title": "Neu starten um Konfiguration anzuwenden",
                    "reboot_content": "Die geänderten WLAN-Einstellungen werden nur nach einem Neustart angewendet. Jetzt neu starten?",
                    "abort": "Abbrechen",
                    "reboot": "Neu starten",
                },
                "script": {
                    "select_ap": "Gefundene Access Points",
                    "no_ap_found": "Kein Netzwerk gefunden.",
                    "ssid": "Netzwerkname",
                    "rssi": "Empfang",
                    "password": "Verschlüsselt",
                    "connect": "Verbinden",
                    "hidden_ap": "Versteckter AP",
                    "passphrase": "Passphrase",
                    "show": "Anzeigen",
                    "passphrase_requirements": "Die Passphrase muss zwischen 8 und 63 ASCII-Zeichen lang sein.",
                    "bssid_lock": "BSSID-Sperre",
                    "bssid_lock_desc_pre": "Verbinde nur zum Access Point mit der BSSID",
                    "bssid_lock_desc_post": ".<br/>Deaktiviert lassen, falls mehrere Access Points oder Repeater mit dem selben Netzwerknamen verwendet werden.",
                    "connect_to": "Verbindung zu",
                    "ip_configuration": "IP-Konfiguration",
                    "dhcp": "Automatisch (DHCP)",
                    "static": "Statisch",
                    "static_title": "Statische IP-Konfiguration",
                    "static_ip": "IP",
                    "subnet_mask": "Subnetzmaske",
                    "gateway": "Gateway",
                    "dns": "DNS-Server",
                    "dns2": "Alternativer DNS-Server",
                    "static_ip_invalid": "Die IP muss aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 10.0.0.2",
                    "subnet_mask_invalid": "Die Subnetzmaske muss aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 255.255.255.0",
                    "gateway_invalid": "Die IP des Gateways muss aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 10.0.0.1",
                    "dns_invalid": "Die IP des DNS-Servers muss entweder leer bleiben, oder aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 10.0.0.1.",
                    "dns2_invalid": "Die IP des alternativen DNS-Servers muss entweder leer bleiben, oder aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 10.0.0.1.",
                    "next": "Weiter",
                    "sta_on_boot_pre": "Verbinde beim Start zu",
                    "sta_on_boot_post": " ",
                    "ap_as_fallback_pre": "Öffne eigenen Accesspoint nur, falls die Verbindung zu",
                    "ap_as_fallback_post": "fehlschlägt",
                    "sta_config_failed": "Speichern der Verbindungseinstellungen fehlgeschlagen.",
                    "config_failed": "Speichern der WLAN-Starteinstellungen fehlgeschlagen.",
                    "wifi_connection": "WLAN-Verbindung",
                    "wifi_connection_pre": "WLAN-Verbindung zu",
                    "no_ap_configured": "Kein Access Point konfiguriert"
                }
            }
        },
        "en": {
            "wifi": {
                "status": {
                    "wifi_connection": "WiFi Connection",
                    "not_configured": "Not configured",
                    "not_connected": "Not connected",
                    "connecting": "Connecting",
                    "connected": "Connected",
                    "wifi_ap": "WiFi Access Point",
                    "deactivated": "Deactivated",
                    "activated": "Activated",
                    "fallback_inactive": "Fallback inactive",
                    "fallback_active": "Fallback active"
                },
                "navbar": {
                    "wifi": "WiFi"
                },
                "content": {
                    "wifi_settings": "WiFi Settings",
                    "cancel": "Cancel",
                    "back": "Back",
                    "client_configuration": "Connection configuration",
                    "configured_ap": "Configured Access Point",
                    "configure_connection": "Configure connection to another access point",
                    "connect_to_wifi": "Connect to WiFi",
                    "scanning_for_aps": "Scanning for Access Points...",
                    "scanning": "Scanne...",
                    "configure_ap_mode": "WiFi Start-Up Settings",
                    "sta_on_boot": "Connect to access point on start-up",
                    "ap_on_boot": "Open local access point on start-up",
                    "ap_as_fallback": "Open local access point only if connection to the access point fails",
                    "finish": "Finish",
                    "reboot_title": "Reboot to apply configuration",
                    "reboot_content": "The changed WiFi configuration will only be applied after rebooting. Reboot now?",
                    "abort": "Abort",
                    "reboot": "Reboot",
                },
                "script": {
                    "select_ap": "Found Access Points",
                    "no_ap_found": "No Wifi networks found.",
                    "ssid": "Network name",
                    "rssi": "Reception",
                    "password": "Encrypted",
                    "connect": "Connect",
                    "hidden_ap": "Hidden AP",
                    "passphrase": "Passphrase",
                    "show": "Show",
                    "passphrase_requirements": "The passphrase must be 8-63 ASCII characters.",
                    "bssid_lock": "BSSID Lock",
                    "bssid_lock_desc_pre": "Connect only to the access point with the BSSID",
                    "bssid_lock_desc_post": ".<br/>Leave deactivated if you use multiple access points or repeaters with with the same SSID.",
                    "connect_to": "Connect to",
                    "ip_configuration": "IP Configuration",
                    "dhcp": "Automatic (DHCP)",
                    "static": "Static",
                    "static_title": "Static IP Configuration",
                    "static_ip": "IP",
                    "subnet_mask": "Subnet Mask",
                    "gateway": "Gateway",
                    "dns": "DNS Server",
                    "dns2": "Alternative DNS Server",
                    "static_ip_invalid": "The IP must have four groups between 0 and 255 separated with a dot, for example: 10.0.0.2",
                    "subnet_mask_invalid": "The subnet mask must have four groups between 0 and 255 separated with a dot, for example: 255.255.255.0",
                    "gateway_invalid": "The gateway IP must have four groups between 0 and 255 separated with a dot, for example: 10.0.0.1",
                    "dns_invalid": "The DNS server IP must have four groups between 0 and 255 separated with a dot, for example: 10.0.0.1, or be empty.",
                    "dns2_invalid": "The alternative DNS server IP must have four groups between 0 and 255 separated with a dot, for example: 10.0.0.1, or be empty.",
                    "next": "Next",
                    "sta_on_boot_pre": "Connect to",
                    "sta_on_boot_post": "on start-up",
                    "ap_as_fallback_pre": "Open local access point only if connection to",
                    "ap_as_fallback_post": "fails",
                    "sta_config_failed": "Failed to save access point configuration.",
                    "config_failed": "Failed to save wifi configuration.",
                    "wifi_connection": "WiFi Connection",
                    "wifi_connection_pre": "WiFi Connection to",
                    "no_ap_configured": "No access point configured"
                }
            }
        }
    }[lang];
}
