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

function upload(e: JQuery.SubmitEvent, type: string, name: string) {
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
                util.show_alert("alert-success", `Successfully flashed ${name}.`, "Rebooting... Please reload the web interface in about 30 seconds.");
            },
            400: function (result) {
                progress.prop("hidden", true);
                util.show_alert("alert-danger", `Failed to flash ${name}.`, result.responseText);
            }
        }
    });
}

export function init() {
    //Firmware upload
    $('#upload_firmware_form').on("submit", function (e) {
        upload(e, "firmware", "Firmware");
    });

    //SPIFFS upload
    $('#upload_spiffs_form').on("submit", function (e) {
        upload(e, "spiffs", "SPIFFS");
    });

    // TODO: use $
    (<HTMLInputElement>document.getElementById("firmware_file_select")).addEventListener("change", () => $("#update_firmware_button").prop("disabled", false));

    (<HTMLInputElement>document.getElementById("spiffs_file_select")).addEventListener("change", () => $("#update_spiffs_button").prop("disabled", false));

    bsCustomFileInput.init();
}

export function addEventListeners(source: EventSource) {
    source.addEventListener('version', function (e: util.SSE) {
        update_version(<Version>(JSON.parse(e.data)));
    }, false);
}
