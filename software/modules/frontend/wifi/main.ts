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
    $("#start_wifi_scan").html("<span class=\"spinner-grow spinner-grow-sm\" role=\"status\" aria-hidden=\"true\"></span> Scanning for APs...");
    $("#start_wifi_scan").prop('disabled', true);

    $.get("/scan_wifi").done(function () {
        $("#wifi_config_scan_spinner").prop('hidden', false);
        $("#wifi_scan_results").prop('hidden', true);
        $("#wifi_scan_title").html("<h4>Scanning for Access Points...</h4>");
        $("#wifi_config_carousel").carousel(1);

        setTimeout(function () {
            $.get("/get_wifis").done(function (data: GetWifiResult) {
                $("#wifi_config_scan_spinner").prop('hidden', true);
                $("#wifi_scan_results").prop('hidden', false);
                $("#wifi_scan_title").html("<h4>Select Access Point</h4>");
                //$("#start_wifi_scan").html("Configure client");
                //$("#start_wifi_scan").prop('disabled', false);
                if (data.result.length == 0) {
                    $("#wifi_scan_results").html("No Wifi networks found.");
                    return;
                }
                let result = `<div class="table-responsive col-lg-6"><table id="wifi_config_found_aps" class="table table-hover">
                <thead>
                    <tr>
                    <th scope="col">SSID</th>
                    <th scope="col">RSSI</th>
                    <th scope="col">Password</th>
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
                    line += `<td><button id="wifi_scan_result_${i}" type="button" class="btn btn-primary">Connect</button></td>`
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
        $('#wifi_sta_connection_name').html('<span class="form-label">WiFi Connection </span>');
    else
        $('#wifi_sta_connection_name').html('<span class="form-label">WiFi Connection to ' + config.ssid + '</span>');

    if (config.ssid != "") {
        $('#wifi_ap_configuration_state').val(config.ssid);
        //$('#wifi_save_enable_sta').removeProp("disabled");
        //$('#wifi_save_ap_fallback_only').removeProp("disabled");
    }
    else {
        $('#wifi_ap_configuration_state').val("No Access Point configured.");
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
    let form_ssid = `<span id="wifi_cfg_ssid"></span>Hidden AP (<span id="wifi_cfg_bssid">${bssid}</span>)`;

    if (encryption != 0) {
        form_passphrase = `<div class="form-group row no-gutters" id="wifi_cfg_passphrase_form">
                <label for="wifi_cfg_passphrase" class="col-sm-3 col-form-label">Passphrase</label>
                <div class="col-sm-9">
                    <div class="input-group">
                        <input type="password" id="wifi_cfg_passphrase" class="form-control" value="" required minlength="8" maxlength="63"/>
                        <div class="input-group-append">
                            <div class="input-group-text custom-control custom-switch" style="padding-left: 2.5rem;">
                                <input id="wifi_cfg_show_passphrase" type="checkbox" class="custom-control-input" aria-label="Show passphrase" value="test">
                                <label class="custom-control-label" for="wifi_cfg_show_passphrase">Show</label>
                            </div>
                        </div>
                        <div class="invalid-feedback">
                            The passphrase must be 8-63 ASCII characters.
                        </div>
                    </div>
                </div>
            </div>`;
    }

    if (ssid != "") {
        form_bssid_lock = `<div class="form-group row no-gutters">
                <label for="wifi_cfg_bssid_lock" class="col-sm-3 col-form-label">BSSID Lock</label>
                <div class="col-sm-9">
                    <div class="custom-control custom-switch">
                        <input type="checkbox" class="custom-control-input" id="wifi_cfg_bssid_lock">
                        <label class="custom-control-label" for="wifi_cfg_bssid_lock">
                            Connect only to the access point with the BSSID ${bssid}.<br/>Leave deactivated if you use multiple
                            access points or repeaters with with the same SSID.
                        </label>
                    </div>
                </div>
            </div>`;
        form_ssid = `<span id="wifi_cfg_ssid">${ssid}</span> (<span id="wifi_cfg_bssid">${bssid}</span>)`;
    }

    let form_content = `
    <div class="d-flex justify-content-between flex-wrap flex-md-nowrap align-items-center pb-2 mb-3">
        <h4>Connect to ${form_ssid}</h4>
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
                    <label for="wifi_cfg_ip_configuration" class="col-form-label">IP
                        Configuration</label>
                </div>

                <div class="col-sm-9">
                    <select id="wifi_cfg_ip_configuration" class="custom-select">
                        <!-- TODO better toggle behaviour-->
                        <option value="hide" selected>DHCP</option>
                        <option value="show">Static</option>
                    </select>
                </div>
            </div>
            <div class="collapse form-group row no-gutters justify-content-end"
                id="wifi_cfg_static_ip_cfg">
                <div class="card col-sm-9">
                    <div class="card-body">
                        <h5 class="card-title">Static IP Configuration</h5>
                        <div class="form-group row justify-content-end">
                            <label for="wifi_cfg_static_ip_ip"
                                class="col-sm-3 col-form-label">IP</label>
                            <div class="col-sm-9">
                                <input id="wifi_cfg_static_ip_ip" class="form-control" type="text"
                                    minlength="7" maxlength="15" size="15"
                                    pattern="^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$">
                                <div class="invalid-feedback">
                                    The IP must have four groups between 0 and 255 separated with a dot, for example: 10.0.0.2
                                </div>
                            </div>
                        </div>

                        <div class="form-group row justify-content-end">
                            <label for="wifi_cfg_static_ip_subnet"
                                class="col-sm-3 col-form-label">Subnet Mask</label>
                            <div class="col-sm-9">
                                <input id="wifi_cfg_static_ip_subnet" class="form-control" type="text"
                                    minlength="7" maxlength="15" size="15"
                                    pattern="^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$">
                                <div class="invalid-feedback">
                                    The Subnet mask must have four groups between 0 and 255 separated with a dot, for example: 255.255.255.0
                                </div>
                            </div>
                        </div>

                        <div class="form-group row justify-content-end">
                            <label for="wifi_cfg_static_ip_gateway"
                                class="col-sm-3 col-form-label">Gateway</label>
                            <div class="col-sm-9">
                                <input id="wifi_cfg_static_ip_gateway" class="form-control" type="text"
                                    minlength="7" maxlength="15" size="15"
                                    pattern="^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$">
                                <div class="invalid-feedback">
                                    The gateway IP must have four groups between 0 and 255 separated with a dot, for example: 10.0.0.2
                                </div>
                            </div>
                        </div>

                        <div class="form-group row justify-content-end">
                            <label for="wifi_cfg_static_ip_dns" class="col-sm-3 col-form-label">DNS
                                Server</label>
                            <div class="col-sm-9">
                                <input id="wifi_cfg_static_ip_dns" class="form-control" type="text"
                                    minlength="7" maxlength="15" size="15"
                                    pattern="^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$">
                                <div class="invalid-feedback">
                                    The DNS IP must have four groups between 0 and 255 separated with a dot, for example: 10.0.0.2
                                </div>
                            </div>
                        </div>

                        <div class="form-group row justify-content-end">
                            <label for="wifi_cfg_static_ip_dns2" class="col-sm-3 col-form-label">DNS Server
                                (Alternative)</label>
                            <div class="col-sm-9">
                                <input id="wifi_cfg_static_ip_dns2" class="form-control" type="text"
                                    minlength="7" maxlength="15" size="15"
                                    pattern="^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$">
                                <div class="invalid-feedback">
                                    The alternative DNS IP must have four groups between 0 and 255 separated with a dot, for example: 10.0.0.2
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
        <button id="wifi_config_submit_button" type="submit" form="wifi_config_form" class="btn btn-primary">Next</button>
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
        $('#wifi_config_connect_to_ap_switch').html("Connect to " + $('#wifi_cfg_ssid').text() + " on start-up.");

        $('#wifi_config_ap_fallback_switch').html("Open local access point only if connection to " + $('#wifi_cfg_ssid').text() + " fails.");
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
        passphrase: $('#wifi_cfg_passphrase').val().toString(),
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
        error: (_x, _y, error) => util.show_alert("alert-danger", "Failed to save access point configuration.", error)
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
        error: (_x, _y, error) => util.show_alert("alert-danger", "Failed to save wifi configuration.", error)
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
