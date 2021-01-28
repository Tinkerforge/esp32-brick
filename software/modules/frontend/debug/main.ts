import $ from "jquery";

import * as util from "../util";

interface DebugState {
    uptime: number,
    free_heap: number,
    largest_free_heap_block: number
}

function update_debug_state(state: DebugState) {
    $('#debug_uptime').val(util.format_timespan(Math.round(state.uptime / 1000)));
    $('#debug_heap_free').val(state.free_heap);
    $('#debug_heap_block').val(state.largest_free_heap_block);
}

export function init() {

}

export function addEventListeners(source: EventSource) {
    source.addEventListener('debug/state', function (e: util.SSE) {
        update_debug_state(<DebugState>(JSON.parse(e.data)));
    }, false);
}

export function updateLockState(module_init) {
    $('#sidebar-debug').prop('hidden', !module_init.debug);
}

export function getTranslation(lang: string) {
    return {
        "de": {
            "debug": {
                "status": {
                    "debug": "Debug"
                },
                "navbar": {
                    "debug": "Debug"
                },
                "content": {
                    "debug": "Debug",
                    "uptime": "Zeit seit Neustart",
                    "heap_free": "Freie Heap-Bytes",
                    "heap_block": "Größter freier Heap-Block",
                },
                "script": {
                }
            }
        },
        "en": {
            "debug": {
                "status": {
                    "debug": "Debug"
                },
                "navbar": {
                    "debug": "Debug"
                },
                "content": {
                    "debug": "Debug",
                    "uptime": "Uptime",
                    "heap_free": "Free heap bytes",
                    "heap_block": "Largest free heap block",
                },
                "script": {
                }
            }
        }
    }[lang];
}
