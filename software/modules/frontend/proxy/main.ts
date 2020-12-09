import $ from "jquery";

import * as util from "../util";

interface Device {
    uid: string,
    port: string,
    name: string
}

function update_devices(devices: Device[]) {
    if (devices.length == 0) {
        $("#bricklets_content").html("No bricklets found.");
        return;
    }

    let result = `<div class="table-responsive col-lg-6"><table id="" class="table table-hover">
    <thead>
        <tr>
        <th scope="col">Port</th>
        <th scope="col">UID</th>
        <th scope="col">Device Type</th>
        </tr>
    </thead>
    <tbody>`;

    for (let device of devices) {
        let line = "<tr>";
        line += "<td>" + device.port + "</td>";
        line += "<td>" + device.uid + "</td>";
        line += "<td>" + device.name + "</td>";
        line += "</tr>"

        result += line;
    }

    result += "</tbody></table></div>";
    $("#bricklets_content").html(result);
}

interface ErrorCounter {
    SpiTfpChecksum: number,
    SpiTfpFrame: number,
    TfpFrame: number,
    TfpUnexpected: number
}

function update_error_counters(error_counters: Map<String, ErrorCounter>) {
    if (error_counters.size == 0) {
        $("#bricklets_error_counters").html("");
        return;
    }

    let result = `<div class="table-responsive col-lg-6"><table class="table table-hover">
    <thead>
        <tr>
            <th scope="col">Port</th>
            <th scope="col">SPI TFP Checksum</th>
            <th scope="col">SPI TFP Frame</th>
            <th scope="col">TFP Frame</th>
            <th scope="col">TFP Unexpected</th>
        </tr>
    </thead>
    <tbody>`;

    for (let port in error_counters) {
        let counters = error_counters[port];

        let line = "<tr>";
        line += "<td>" + port + "</td>";
        line += "<td>" + counters.SpiTfpChecksum + "</td>";
        line += "<td>" + counters.SpiTfpFrame + "</td>";
        line += "<td>" + counters.TfpFrame + "</td>";
        line += "<td>" + counters.TfpUnexpected + "</td>";
        line += "</tr>"

        result += line;
    }

    result += "</tbody></table></div>";
    $("#bricklets_error_counters").html(result);
}

export function addEventListeners(source: EventSource) {
    source.addEventListener('devices', function (e: util.SSE) {
        update_devices(<Device[]>(JSON.parse(e.data)));
    }, false);

    source.addEventListener('bricklet_error_counters', function (e: util.SSE) {
        update_error_counters(<Map<String, ErrorCounter>>(JSON.parse(e.data)));
    }, false);
}

export function init() {

}


export function updateLockState(module_init) {
    $('#sidebar-bricklets').prop('hidden', !module_init.proxy);
}
