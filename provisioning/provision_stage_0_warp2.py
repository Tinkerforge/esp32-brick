#!/usr/bin/python3 -u

import contextlib
from contextlib import contextmanager
import datetime
import io
import json
import os
import re
import secrets
import shutil
import socket
import ssl
import subprocess
import sys
import tempfile
import threading
import time
import urllib.request

from tinkerforge.ip_connection import IPConnection, base58encode, base58decode, BASE58
from tinkerforge.bricklet_rgb_led_v2 import BrickletRGBLEDV2

from provision_common.provision_common import *

def main():
    if len(sys.argv) != 3:
        print("Usage: {} test_firmware port")
        sys.exit(0)

    if not os.path.exists(sys.argv[1]):
        print("Test firmware {} not found.".format(sys.argv[1]))

    PORT = sys.argv[2]

    common_init(PORT, None, None)

    if not os.path.exists(PORT):
        print("Port {} not found.".format(PORT))

    result = {"start": now()}

    print("Checking ESP state")
    mac_address = check_if_esp_is_sane_and_get_mac()
    print("MAC Address is {}".format(mac_address))
    result["mac"] = mac_address

    set_voltage_fuses, set_block_3, passphrase, uid = get_espefuse_tasks()
    result["set_voltage_fuses"] = set_voltage_fuses
    result["set_block_3"] = set_block_3

    handle_voltage_fuses(set_voltage_fuses)

    uid, passphrase = handle_block3_fuses(set_block_3, uid, passphrase)

    print("Verifying eFuses")
    _set_voltage_fuses, _set_block_3, _passphrase, _uid = get_espefuse_tasks()
    if _set_voltage_fuses:
        print("Failed to verify voltage eFuses! Are they burned in yet?")
        sys.exit(0)

    if _set_block_3:
        print("Failed to verify block 3 eFuses! Are they burned in yet?")
        sys.exit(0)

    if _passphrase != passphrase:
        print("Failed to verify block 3 eFuses! Passphrase is not the expected value")
        sys.exit(0)

    if _uid != uid:
        print("Failed to verify block 3 eFuses! UID {} is not the expected value {}".format(_uid, uid))
        sys.exit(0)

    result["uid"] = uid

    print("Erasing flash")
    erase_flash()

    print("Flashing test firmware")
    flash_firmware(sys.argv[1])
    result["test_firmware"] = sys.argv[1]

    ssid = "warp2-" + uid

    result["end"] = now()

    with open("{}_{}_report_stage_0.json".format(ssid, now().replace(":", "-")), "w") as f:
        json.dump(result, f, indent=4)

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        traceback.print_exc()
        input("Press return to exit. ")
        sys.exit(1)
