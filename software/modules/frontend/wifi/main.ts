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

function wifi_symbol(rssi) {
    if(rssi >= -60)
        return '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><path d="M1.42 9a16 16 0 0 1 21.16 0"></path><path d="M5 12.55a11 11 0 0 1 14.08 0"></path><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>';
    if(rssi >= -70)
        return '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><path d="M5 12.55a11 11 0 0 1 14.08 0"></path><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>';
    if(rssi >= -80)
        return '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><path d="M8.53 16.11a6 6 0 0 1 6.95 0"></path><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>';
    return '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-wifi"><line x1="12" y1="20" x2="12.01" y2="20"></line></svg>';
}

function scan_wifi() {
    $.ajax({
        url: '/wifi/scan',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(null),
        error: (xhr, status, error) => util.show_alert("alert-danger", __("wifi.script.scan_wifi_failed"), error + ": " + xhr.responseText),
        success: () => {
            setTimeout(function () {
                    $.get("/wifi/scan_results").done(function (data: GetWifiResult) {
                        $("#wifi_config_scan_spinner").prop('hidden', true);
                        $("#wifi_scan_results").prop('hidden', false);
                        $("#wifi_scan_title").html(__("wifi.script.select_ap"));

                        if (data.result.length == 0) {
                            $("#wifi_scan_results").html(__("wifi.script.no_ap_found"));
                            return;
                        }
                        let result = ``;

                        $.each(data.result, (i, v: WifiInfo) => {
                            let line = `<a id="wifi_scan_result_${i}" class="dropdown-item" href="#">${wifi_symbol(v.rssi)}<span data-feather='${v.encryption == 0 ? 'unlock' : 'lock'}'></span><span class="pl-2">${v.ssid == "" ? __("wifi.script.hidden_ap") + ` (${v.bssid})` : v.ssid}</span></a>`;
                            result += line;
                        });

                        $("#wifi_scan_results").html(result);
                        $("#scan_wifi_button").dropdown('update')

                        $.each(data.result, (i, v: WifiInfo) => {
                            let button = document.getElementById(`wifi_scan_result_${i}`);
                            button.addEventListener("click", () => connect_to_ap(v.ssid, v.bssid, v.encryption));
                        });

                        feather.replace();
                    });
                }, 5000);
        }
    });
}

interface WifiSTAConfig {
    enable_sta: boolean,
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
    $('#wifi_sta_enable_sta').prop("checked", config.enable_sta);

    if (config.ssid == "")
        $('#wifi_sta_connection_name').html(`<span class="form-label">${__("wifi.script.wifi_connection")}</span>`);
    else
        $('#wifi_sta_connection_name').html(`<span class="form-label">${__("wifi.script.wifi_connection_pre")} ${config.ssid}</span>`);

    if (config.ssid != "") {
        $('#wifi_ap_configuration_state').val(config.ssid);
    }
    else {
        $('#wifi_ap_configuration_state').val(__("wifi.script.no_ap_configured"));
    }

    $('#wifi_sta_ssid').val(config.ssid);
    $('#wifi_sta_bssid').val(config.bssid.map((x)=> (x < 16 ? '0' : '') + x.toString(16).toUpperCase()).join(":"));
    $('#wifi_sta_bssid_lock').prop("checked", config.bssid_lock);
    $('#wifi_sta_passphrase').val(config.passphrase);
    $('#wifi_sta_hostname').val(config.hostname);
    if(config.ip.join(".") == "0.0.0.0") {
        $('#wifi_sta_show_static').val("hide");
    } else {
        $('#wifi_sta_show_static').val("show");
    }
    $('#wifi_sta_ip').val(config.ip.join("."));
    $('#wifi_sta_gateway').val(config.gateway.join("."));
    $('#wifi_sta_subnet').val(config.subnet.join("."));
    $('#wifi_sta_dns').val(config.dns.join("."));
    $('#wifi_sta_dns2').val(config.dns2.join("."));
}

interface WifiAPConfig {
    enable_ap: boolean,
    ap_fallback_only: boolean,
    ssid: string,
    hide_ssid: boolean,
    passphrase: string,
    hostname: string,
    channel: number,
    ip: number[],
    gateway: number[],
    subnet: number[]
}

function update_wifi_ap_config(config: WifiAPConfig) {
    $('#wifi_ap_ssid').val(config.ssid);
    $('#wifi_ap_hide_ssid').prop("checked", config.hide_ssid);
    $('#wifi_ap_passphrase').val(config.passphrase);
    $('#wifi_ap_hostname').val(config.hostname);
    $('#wifi_ap_channel').val(config.channel);
    $('#wifi_ap_ip').val(config.ip.join("."));
    $('#wifi_ap_gateway').val(config.gateway.join("."));
    $('#wifi_ap_subnet').val(config.subnet.join("."));

    if(config.enable_ap && config.ap_fallback_only)
        $('#wifi_ap_enable_ap').val(1);
    else if(config.enable_ap)
        $('#wifi_ap_enable_ap').val(0);
    else
        $('#wifi_ap_enable_ap').val(2);

    $('#wifi_save_enable_ap').prop("checked", config.enable_ap);
    $('#wifi_save_ap_fallback_only').prop("checked", config.ap_fallback_only);
}

interface WifiState {
    connection_state: number,
    ap_state: number,
    ap_bssid: string
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

    $('#wifi_ap_bssid').html(state.ap_bssid);
}

function wifi_cfg_toggle_static_ip_collapse(value: string) {
    if (value == "hide") {
        $('#wifi_sta_static_ip_cfg').collapse('hide');
        $('#wifi_sta_ip').prop('required', false);
        $('#wifi_sta_subnet').prop('required', false);
        $('#wifi_sta_gateway').prop('required', false);
    }
    else if (value == "show") {
        $('#wifi_sta_static_ip_cfg').collapse('show');
        $('#wifi_sta_ip').prop('required', true);
        $('#wifi_sta_subnet').prop('required', true);
        $('#wifi_sta_gateway').prop('required', true);
    }
}

function connect_to_ap(ssid: string, bssid: string, encryption: number) {
    $('#wifi_sta_ssid').val(ssid);
    $('#wifi_sta_bssid').val(bssid);
    $('#wifi_sta_passphrase').prop("required", encryption != 0);
    $('#wifi_sta_enable_sta').prop("checked", true);
    $('#wifi_sta_bssid_lock').prop("checked", ssid == "");
    return;
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
        enable_sta: $('#wifi_sta_enable_sta').is(':checked'),
        hostname: $('#wifi_sta_hostname').val(),
        ssid: $('#wifi_sta_ssid').val(),
        bssid: $('#wifi_sta_bssid').val().split(':').map(x => parseInt(x, 16)),
        bssid_lock: $('#wifi_sta_bssid_lock').is(':checked'),
        passphrase: $('#wifi_sta_passphrase').val().length > 0 ? $('#wifi_sta_passphrase').val().toString() : null,
        ip: dhcp ? [0, 0, 0, 0] : parse_ip($('#wifi_sta_ip').val().toString()),
        subnet: dhcp ? [0, 0, 0, 0] : parse_ip($('#wifi_sta_subnet').val().toString()),
        gateway: dhcp ? [0, 0, 0, 0] : parse_ip($('#wifi_sta_gateway').val().toString()),
        dns: dhcp ? [0, 0, 0, 0] : parse_ip($('#wifi_sta_dns').val().toString()),
        dns2: dhcp ? [0, 0, 0, 0] : parse_ip($('#wifi_sta_dns2').val().toString())
    };

    $.ajax({
        url: '/wifi/sta_config_update',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(payload),
        success: continuation,
        error: (xhr, status, error) => util.show_alert("alert-danger", __("wifi.script.sta_config_failed"), error + ": " + xhr.responseText)
    });
}

function save_wifi_ap_config(continuation = function () { }) {

    let payload: WifiAPConfig = {
        enable_ap: $('#wifi_ap_enable_ap').val() != 2,
        ap_fallback_only: $('#wifi_ap_enable_ap').val() == 1,
        ssid: $('#wifi_ap_ssid').val(),
        hide_ssid: $('#wifi_ap_hide_ssid').is(':checked'),
        passphrase: $('#wifi_ap_passphrase').val().length > 0 ? $('#wifi_ap_passphrase').val().toString() : null,
        hostname: $('#wifi_ap_hostname').val(),
        channel: parseInt($('#wifi_ap_channel').val()),
        ip: parse_ip($('#wifi_ap_ip').val().toString()),
        subnet: parse_ip($('#wifi_ap_subnet').val().toString()),
        gateway: parse_ip($('#wifi_ap_gateway').val().toString()),
    };

    $.ajax({
        url: '/wifi/ap_config_update',
        method: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify(payload),
        success: continuation,
        error: (xhr, status, error) => util.show_alert("alert-danger", __("wifi.script.ap_config_failed"), error + ": " + xhr.responseText),
    });
}

function wifi_save_reboot() {
    $('#wifi_reboot').modal('hide');
    util.reboot();
}

export function addEventListeners(source: EventSource) {
    source.addEventListener('wifi/state', function (e: util.SSE) {
        update_wifi_state(<WifiState>(JSON.parse(e.data)));
    }, false);

    source.addEventListener('wifi/sta_config', function (e: util.SSE) {
        update_wifi_sta_config(<WifiSTAConfig>(JSON.parse(e.data)));
    }, false);

    source.addEventListener('wifi/ap_config', function (e: util.SSE) {
        update_wifi_ap_config(<WifiAPConfig>(JSON.parse(e.data)));
    }, false);

}

export function init() {
    (<HTMLButtonElement>document.getElementById("scan_wifi_button")).addEventListener("click", scan_wifi);

    (<HTMLButtonElement>document.getElementById("wifi_reboot_button")).addEventListener("click", wifi_save_reboot);

    let sta_show_pw = <HTMLButtonElement>document.getElementById("wifi_sta_show_passphrase");
    sta_show_pw.addEventListener("change", util.toggle_password_fn("#wifi_sta_passphrase"));

    let ap_show_pw = <HTMLButtonElement>document.getElementById("wifi_ap_show_passphrase");
    ap_show_pw.addEventListener("change", util.toggle_password_fn("#wifi_ap_passphrase"));

    let ap_ip_config = <HTMLInputElement>document.getElementById("wifi_sta_show_static");
    ap_ip_config.addEventListener("change", () => wifi_cfg_toggle_static_ip_collapse(ap_ip_config.value));

    // Use bootstrap form validation
    let form = <HTMLFormElement>$('#wifi_sta_form')[0];
    form.addEventListener('submit', function (event: Event) {
        form.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (form.checkValidity() === false) {
            return;
        }
        save_wifi_sta_config(() => $('#wifi_reboot').modal('show'));
    }, false);

    let form2 = <HTMLFormElement>$('#wifi_ap_form')[0];
    form2.addEventListener('submit', function (event: Event) {
        form2.classList.add('was-validated');
        event.preventDefault();
        event.stopPropagation();

        if (form2.checkValidity() === false) {
            return;
        }
        save_wifi_ap_config(() => $('#wifi_reboot').modal('show'));
    }, false);

    $('#scan_wifi_dropdown').on('hidden.bs.dropdown', function (e) {
        $("#wifi_scan_title").html(__("wifi.content.sta_scanning"));
        $("#wifi_config_scan_spinner").prop('hidden', false);
        $("#wifi_scan_results").prop('hidden', true);
        $("#scan_wifi_button").dropdown('update');
    });
}


export function updateLockState(module_init) {
    $('#sidebar-wifi-sta').prop('hidden', !module_init.wifi);
    $('#sidebar-wifi-ap').prop('hidden', !module_init.wifi);
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
                    "wifi_sta": "WLAN-Verbindung",
                    "wifi_ap": "WLAN-Access-Point",
                },
                "content": {
                    "sta_settings": "WLAN-Verbindungs&shy;einstellungen",
                    "client_configuration": "Verbindungs&shy;einstellungen",
                    "sta_enable_sta": "Verbindung zu Netzwerk aktivieren",
                    "sta_enable_sta_desc": "Wenn aktiviert, verbindet sich das Gerät beim Start automatisch zum konfigurierten Netzwerk.",
                    "sta_ssid": "Netzwerkname (SSID)",
                    "sta_scan": "Netzwerksuche",
                    "sta_scanning": "Scanne Netzwerke...",
                    "sta_bssid": "BSSID",
                    "sta_bssid_lock": "BSSID-Sperre",
                    "sta_bssid_invalid": "Die BSSID muss aus sechs Gruppen mit jeweils einer zweistelligen Hexadezimalzahl, getrennt durch einen Doppelpunkt, bestehen. Zum Beispiel 01:23:45:67:89:AB",
                    "sta_bssid_lock_desc": "Verbinde nur zum Netzwerk mit der konfigurierten BSSID. Deaktiviert lassen, falls mehrere Access Points oder Repeater mit dem selben Netzwerknamen verwendet werden.",
                    "sta_passphrase": "Passphrase",
                    "sta_show_passphrase": "Anzeigen",
                    "sta_hostname": "Hostname",
                    "sta_hostname_invalid": "Der Hostname darf nur aus den Groß- und Kleinbuchstaben A-Z und a-z, sowie den Ziffern 0-9 und Bindestrichen bestehen. Zum Beispiel: warp-A1c",
                    "sta_ip_configuration": "IP-Konfiguration",
                    "sta_dhcp": "Automatisch (DHCP)",
                    "sta_static": "Statisch",
                    "save": "Speichern",

                    "reboot_title": "Neu starten um Konfiguration anzuwenden",
                    "reboot_content": "Die geänderten WLAN-Einstellungen werden nur nach einem Neustart angewendet. Jetzt neu starten?",
                    "abort": "Abbrechen",
                    "reboot": "Neu starten",

                    "static_title": "Statische IP-Konfiguration",
                    "static_ip": "IP-Addresse",
                    "subnet": "Subnetzmaske",
                    "gateway": "Gateway",
                    "dns": "DNS-Server",
                    "dns2": "Alternativer DNS-Server",
                    "static_ip_invalid": "Die IP muss aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 10.0.0.2",
                    "subnet_invalid": "Die Subnetzmaske muss aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 255.255.255.0",
                    "gateway_invalid": "Die IP des Gateways muss aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 10.0.0.1",
                    "dns_invalid": "Die IP des DNS-Servers muss entweder leer bleiben, oder aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 10.0.0.1.",
                    "dns2_invalid": "Die IP des alternativen DNS-Servers muss entweder leer bleiben, oder aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 10.0.0.1.",

                    "ap_settings": "WLAN-Access-Point-Einstellungen",
                    "ap_enable": "Access Point",
                    "ap_enabled": "aktiviert",
                    "ap_fallback_only": "nur als Fallback",
                    "ap_disabled": "deaktiviert",
                    "ap_ssid": "Netzwerkname (SSID)",
                    "ap_hide_ssid": "Netzwerknamen verstecken",
                    "ap_hide_ssid_desc_pre": "Versteckt den Netzwerknamen. Das Gerät ist unter der BSSID ",
                    "ap_hide_ssid_desc_post": " erreichbar.",
                    "ap_passphrase": "Passphrase",
                    "ap_show_passphrase": "Anzeigen",
                    "ap_hostname": "Hostname",
                    "ap_hostname_invalid": "Der Hostname darf nur aus den Groß- und Kleinbuchstaben A-Z und a-z, sowie den Ziffern 0-9 und Bindestrichen bestehen. Zum Beispiel: warp-A1c",
                    "ap_channel": "<span class=\"form-label pr-2\">Kanal</span><span class=\"text-muted\">ignoriert wenn WLAN-Verbindung aktiv ist</span>",
                    "ap_ip": "IP-Addresse",
                    "ap_subnet": "Subnetzmaske",
                    "ap_gateway": "Gateway",
                    "ap_ip_invalid": "Die IP muss aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 10.0.0.2",
                    "ap_subnet_invalid": "Die Subnetzmaske muss aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 255.255.255.0",
                    "ap_gateway_invalid": "Die IP des Gateways muss aus vier Gruppen mit jeweils einer Zahl zwischen 0 und 255, getrent durch einen Punkt, bestehen. Zum Beispiel 10.0.0.1",

                    "unchanged": "Unverändert"
                },
                "script": {
                    "scan_wifi_failed": "Suche nach Netzwerken fehlgeschlagen",
                    "select_ap": "Gefundene Netzwerke",
                    "no_ap_found": "Kein Netzwerk gefunden.",
                    "hidden_ap": "[Versteckter AP]",
                    "passphrase_requirements": "Die Passphrase muss zwischen 8 und 63 ASCII-Zeichen lang sein.",

                    "sta_config_failed": "Speichern der Verbindungseinstellungen fehlgeschlagen.",
                    "ap_config_failed": "Speichern der Access Point-Einstellungen fehlgeschlagen.",
                    "wifi_connection": "WLAN-Verbindung",
                    "wifi_connection_pre": "WLAN-Verbindung zu",
                    "no_ap_configured": "Keine Verbindung konfiguriert"
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
                    "sta_settings": "WiFi Connection Settings",
                    "client_configuration": "Connection configuration",
                    "sta_enable_sta": "Enable connection to network",
                    "sta_enable_sta_desc": "Automatically connects to the configured network if activated.",
                    "sta_ssid": "Network name (SSID)",
                    "sta_scan": "Search for networks",
                    "sta_scanning": "Searching for networks...",
                    "sta_bssid": "BSSID",
                    "sta_bssid_invalid": "The BSSID must have six groups separated by a colon, each with a two-digit hexadecimal number. For example 01:23:45:67:89:AB",
                    "sta_bssid_lock": "BSSID Lock",
                    "sta_bssid_lock_desc": "Connect only to the network with the configured BSSID. Leave deactivated if you use multiple access points or repeaters with with the same network name.",
                    "sta_passphrase": "Passphrase",
                    "sta_show_passphrase": "Show",
                    "sta_hostname": "Hostname",
                    "sta_hostname_invalid": "The hostname must contain only upper and lower case letters A-Z and a-z the digits 0-9 or dashes. For example: warp-A1c",
                    "sta_ip_configuration": "IP Configuration",
                    "sta_dhcp": "Automatic (DHCP)",
                    "sta_static": "Static",
                    "save": "Save",

                    "reboot_title": "Restart to apply configuration",
                    "reboot_content": "The modified WiFi configuration will only be applied after a restart. Reboot now?",
                    "abort": "Abort",
                    "reboot": "Reboot",

                    "static_title": "Static IP Configuration",
                    "static_ip": "IP Address",
                    "subnet": "Subnet Mask",
                    "gateway": "Gateway",
                    "dns": "DNS Server",
                    "dns2": "Alternative DNS Server",
                    "static_ip_invalid": "The IP must have four groups between 0 and 255 separated with a dot, for example: 10.0.0.2",
                    "subnet_invalid": "The subnet mask must have four groups between 0 and 255 separated with a dot, for example: 255.255.255.0",
                    "gateway_invalid": "The gateway IP must have four groups between 0 and 255 separated with a dot, for example: 10.0.0.1",
                    "dns_invalid": "The DNS server IP must have four groups between 0 and 255 separated with a dot, for example: 10.0.0.1, or be empty.",
                    "dns2_invalid": "The alternative DNS server IP must have four groups between 0 and 255 separated with a dot, for example: 10.0.0.1, or be empty.",

                    "ap_settings": "WiFi Access Point Settings",
                    "ap_enable": "Access Point",
                    "ap_enabled": "enabled",
                    "ap_fallback_only": "as fallback only",
                    "ap_disabled": "disabled",
                    "ap_ssid": "Network name (SSID)",
                    "ap_hide_ssid": "Hide network name",
                    "ap_hide_ssid_desc_pre": "Hides the network name. The device is reachable under the BSSID ",
                    "ap_hide_ssid_desc_post": ".",
                    "ap_passphrase": "Passphrase",
                    "ap_show_passphrase": "Show",
                    "ap_hostname": "Hostname",
                    "ap_hostname_invalid": "The hostname must contain only upper and lower case letters A-Z and a-z the digits 0-9 or dashes. For example: warp-A1c",
                    "ap_channel": "<span class=\"form-label pr-2\">Channel</span><span class=\"text-muted\">ignored if WiFi connection is active</span>",
                    "ap_ip": "IP Address",
                    "ap_subnet": "Subnet mask",
                    "ap_gateway": "Gateway",
                    "ap_ip_invalid": "The IP must have four groups between 0 and 255 separated with a dot, for example: 10.0.0.2",
                    "ap_subnet_invalid": "The subnet mask must have four groups between 0 and 255 separated with a dot, for example: 255.255.255.0",
                    "ap_gateway_invalid": "The gateway IP must have four groups between 0 and 255 separated with a dot, for example: 10.0.0.1",

                    "unchanged": "unchanged"
                },
                "script": {
                    "scan_wifi_failed": "Search for networks failed",
                    "select_ap": "Found networks",
                    "no_ap_found": "No network found",
                    "hidden_ap": "[Hidden AP]",
                    "passphrase_requirements": "The passphrase must be 8-63 ASCII characters.",

                    "sta_config_failed": "Failed to save connection configuration.",
                    "ap_config_failed": "Failed to save access point configuration.",
                    "wifi_connection": "WiFi Connection",
                    "wifi_connection_pre": "WiFi Connection to",
                    "no_ap_configured": "No connection configured"
                }
            }
        }
    }[lang];
}
