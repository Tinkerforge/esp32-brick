#!/usr/bin/python3 -u

import contextlib
from contextlib import contextmanager
import datetime
import getpass
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
import csv
import traceback

from tinkerforge.ip_connection import IPConnection, base58encode, base58decode, BASE58
from tinkerforge.bricklet_rgb_led_v2 import BrickletRGBLEDV2

from provision_common.provision_common import *
from provision_common.bricklet_evse_v2 import BrickletEVSEV2
from provision_stage_3_warp2 import Stage3

evse = None

def run_bricklet_tests(ipcon, result, qr_variant, qr_power, ssid, stage3):
    global evse
    enumerations = enumerate_devices(ipcon)

    master = next((e for e in enumerations if e.device_identifier == 13), None)
    evse_enum = next((e for e in enumerations if e.device_identifier == 2167), None)

    if len(enumerations) not in [1, 2]:
        fatal_error("Unexpected number of devices! Expected 1 or 2 but got {}.".format(len(enumerations)))

    if evse_enum is None:
        fatal_error("No EVSE Bricklet found!")

    is_basic = master is not None

    evse = BrickletEVSEV2(evse_enum.uid, ipcon)
    has_meter = evse.get_energy_meter_state().available

    is_smart = not is_basic and not has_meter
    is_pro = not is_basic and has_meter

    d = {"P": "Pro", "S": "Smart", "B": "Basic"}

    if is_basic and qr_variant != "B":
        fatal_error("Scanned QR code implies variant {}, but detected was Basic (i.e. an Master Brick was found)".format(d[qr_variant]))

    if is_smart and qr_variant != "S":
        fatal_error("Scanned QR code implies variant {}, but detected was Smart: An ESP32 Brick was found, not no RS485 Bricklet. Is the Bricklet not connected or the status LED not lighting up? Is the QR code correct?".format(d[qr_variant]))

    if is_pro and qr_variant != "P":
        fatal_error("Scanned QR code implies variant {}, but detected was Pro: An ESP32 Brick and a RS485 Bricklet was found. Is the QR code correct?".format(d[qr_variant]))

    result["evse_uid"] = evse_enum.uid
    print("EVSE UID is {}".format(evse_enum.uid))

    if is_basic:
        result["master_uid"] = master.uid
        print("Master UID is {}".format(master.uid))

    jumper_config, has_lock_switch = evse.get_hardware_configuration()

    if qr_power == "11" and jumper_config != 3:
        fatal_error("Wrong jumper config detected: {} but expected {} as the configured power is {} kW.".format(jumper_config, 3, qr_power))

    if qr_power == "22" and jumper_config != 6:
        fatal_error("Wrong jumper config detected: {} but expected {} as the configured power is {} kW.".format(jumper_config, 6, qr_power))

    result["jumper_config_checked"] = True
    if has_lock_switch:
        fatal_error("Wallbox has lock switch. Is the diode missing?")

    result["diode_checked"] = True

    _configured, _incoming, outgoing, _managed = evse.get_max_charging_current()
    if qr_power == "11" and outgoing != 20000:
        fatal_error("Wrong type 2 cable config detected: Allowed current is {} but expected 20 A, as this is a 11 kW box.".format(outgoing / 1000))
    if qr_power == "22" and outgoing != 32000:
        fatal_error("Wrong type 2 cable config detected: Allowed current is {} but expected 32 A, as this is a 22 kW box.".format(outgoing / 1000))

    result["resistor_checked"] = True

    if is_pro:
        meter_str = urllib.request.urlopen('http://{}/meter/live'.format(ssid), timeout=3).read()
        meter_data = json.loads(meter_str)
        sps = meter_data["samples_per_second"]
        samples = meter_data["samples"]
        if not 0.2 < sps < 2.5:
            fatal_error("Expected between 0.2 and 2.5 energy meter samples per second, but got {}".format(sps))
        if len(samples) < 2:
            fatal_error("Expected at least 10 samples but got {}".format(len(samples)))

        error_count = evse.get_energy_meter_state().error_count
        if any(x != 0 for x in error_count):
            fatal_error("Energy meter error count is {}, expected only zeros!".format(error_count))

        result["energy_meter_reachable"] = True

    stage3.test_front_panel_button()
    result["front_panel_button_tested"] = True


def exists_evse_test_report(evse_uid):
    with open(os.path.join("evse_v2_test_report", "full_test_log.csv"), newline='') as csvfile:
        reader = csv.reader(csvfile, delimiter=',')
        for row in reader:
            if row[0] == evse_uid:
                return True
    return False


def is_front_panel_button_pressed():
    global evse
    return evse.get_low_level_state().gpio[6]

def get_iec_state():
    global evse
    return evse.get_state().iec61851_state

def reset_dc_fault():
    global evse
    return evse.reset_dc_fault_current(0xDC42FA23)

def has_evse_error():
    global evse
    return evse.get_state().error_state != 0

def led_wrap():
    stage3 = Stage3(is_front_panel_button_pressed_function=is_front_panel_button_pressed, get_iec_state_function=get_iec_state, reset_dc_fault_function=reset_dc_fault, has_evse_error=has_evse_error)
    stage3.setup()
    stage3.set_led_strip_color(0, 0, 255)
    try:
        main(stage3)
    except BaseException:
        stage3.set_led_strip_color(255, 0, 0)
        raise
    else:
        stage3.set_led_strip_color(0, 255, 0)

def main(stage3):
    result = {"start": now()}

    git_user = None
    if len(sys.argv) == 2:
        git_user = sys.argv[1]

    with urllib.request.urlopen("https://download.tinkerforge.com/latest_versions.txt") as f:
        latest_versions = f.read().decode("utf-8")

    match = re.search(r"bricklets:evse_v2:(\d)\.(\d)\.(\d)", latest_versions)
    major = match.group(1)
    minor = match.group(2)
    patch = match.group(3)

    if not os.path.exists("firmwares"):
        os.mkdir("firmwares")

    evse_path = "bricklet_evse_v2_firmware_{}_{}_{}.zbin".format(major, minor, patch)
    if not os.path.exists(evse_path):
        urllib.request.urlretrieve('https://download.tinkerforge.com/firmwares/bricklets/evse_v2/{}'.format(evse_path), os.path.join("firmwares", evse_path))
    evse_path = os.path.join("firmwares", evse_path)

    #with urllib.request.urlopen("https://www.warp-charger.com/") as f:
    #    warp_charger_page = f.read().decode("utf-8")

    #match = re.search(r'<a href="firmwares/(warp_firmware_\d_\d_\d_[0-9a-f]{8}_merged.bin)" class="btn btn-primary btn-lg" id="download_latest_firmware">', #warp_charger_page)
    #firmware_path = match.group(1)

    #if not os.path.exists(firmware_path):
    #    urllib.request.urlretrieve('https://www.warp-charger.com/firmwares/{}'.format(firmware_path), os.path.join("firmwares", firmware_path))
    #firmware_path = os.path.join("firmwares", firmware_path)

    #T:WARP2-CP-22KW-50;V:2.1;S:5000000001;B:2021-09;O:SO/2020123;I:17/42;E:1;C:0;;;;;;
    pattern = r'^T:WARP2-C(B|S|P)-(11|22)KW-(50|75);V:(\d+\.\d+);S:(5\d{9});B:(\d{4}-\d{2});O:(SO/B?[0-9]+);I:(\d+/\d+);E:(\d+);C:([01]);;;*$'
    qr_code = my_input("Scan the docket QR code")
    match = re.match(pattern, qr_code)
    while not match:
        qr_code = my_input("Scan the docket QR code", red)
        match = re.match(pattern, qr_code)

    docket_variant = match.group(1)
    docket_power = match.group(2)
    docket_cable_len = match.group(3)
    docket_hw_version = match.group(4)
    docket_serial = match.group(5)
    docket_built = match.group(6)
    docket_order = match.group(7)
    docket_item = match.group(8)
    docket_supply_cable_extension = match.group(9)
    docket_has_cee = match.group(10)

    if docket_supply_cable_extension is None:
        docket_supply_cable_extension = 0
    else:
        docket_supply_cable_extension = int(docket_supply_cable_extension)

    print("Docket QR code data:")
    print("    WARP Charger {}".format({"B": "Basic", "S": "Smart", "P": "Pro"}[docket_variant]))
    print("    {} kW".format(docket_power))
    print("    {:1.1f} m".format(int(docket_cable_len) / 10.0))
    print("    CEE: {}".format("Yes" if docket_has_cee == "1" else "No"))
    print("    HW Version: {}".format(docket_hw_version))
    print("    Serial: {}".format(docket_serial))
    print("    Build month: {}".format(docket_built))
    print("    Order: {}".format(docket_order))
    print("    Item: {}".format(docket_item))
    print("    Supply Cable Extension: {}".format(docket_supply_cable_extension))

    result["order"] = docket_order
    result["order_item"] = docket_item
    result["supply_cable_extension"] = docket_supply_cable_extension
    result["docket_qr_code"] = match.group(0)

    #T:WARP2-CP-22KW-50;V:2.1;S:5000000001;B:2021-09;;
    pattern = r'^T:WARP2-C(B|S|P)-(11|22)KW-(50|75);V:(\d+\.\d+);S:(5\d{9});B:(\d{4}-\d{2});;;*$'
    qr_code = my_input("Scan the wallbox QR code")
    match = re.match(pattern, qr_code)
    while not match:
        qr_code = my_input("Scan the wallbox QR code", red)
        match = re.match(pattern, qr_code)

    qr_variant = match.group(1)
    qr_power = match.group(2)
    qr_cable_len = match.group(3)
    qr_hw_version = match.group(4)
    qr_serial = match.group(5)
    qr_built = match.group(6)

    if docket_variant != qr_variant or \
       docket_power != qr_power or \
       docket_cable_len != qr_cable_len or \
       docket_hw_version != qr_hw_version or \
       docket_serial != qr_serial or \
       docket_built != qr_built:
        fatal_error("Docket and wallbox QR code do not match!")

    print("Wallbox QR code data:")
    print("    WARP Charger {}".format({"B": "Basic", "S": "Smart", "P": "Pro"}[qr_variant]))
    print("    {} kW".format(qr_power))
    print("    {:1.1f} m".format(int(qr_cable_len) / 10.0))
    print("    HW Version: {}".format(qr_hw_version))
    print("    Serial: {}".format(qr_serial))
    print("    Build month: {}".format(qr_built))

    result["serial"] = qr_serial
    result["qr_code"] = match.group(0)

    stage3.power_on({"B": "Basic", "S": "Smart", "P": "Pro"}[qr_variant])

    if qr_variant != "B":
        pattern = r"^WIFI:S:(esp32|warp|warp2)-([{BASE58}]{{3,6}});T:WPA;P:([{BASE58}]{{4}}-[{BASE58}]{{4}}-[{BASE58}]{{4}}-[{BASE58}]{{4}});;$".format(BASE58=BASE58)
        qr_code = getpass.getpass(green("Scan the ESP Brick QR code"))
        match = re.match(pattern, qr_code)
        while not match:
            qr_code = getpass.getpass(red("Scan the ESP Brick QR code"))
            match = re.match(pattern, qr_code)

        hardware_type = match.group(1)
        esp_uid_qr = match.group(2)
        passphrase_qr = match.group(3)

        print("ESP Brick QR code data:")
        print("    Hardware type: {}".format(hardware_type))
        print("    UID: {}".format(esp_uid_qr))

        result["uid"] = esp_uid_qr

        ssid = "warp2-" + esp_uid_qr

        print("Connecting via ethernet to {}".format(ssid), end="")
        for i in range(30):
            start = time.time()
            try:
                with urllib.request.urlopen("http://{}/hidden_proxy/enable".format(ssid), timeout=1) as f:
                    f.read()
                    break
            except:
                pass
            t = max(0, 1 - (time.time() - start))
            time.sleep(t)
            print(".", end="")
        else:
            print("Failed to connect via ethernet!")
            raise Exception("exit 1")
        print(" Connected.")

        ipcon = IPConnection()
        try:
            ipcon.connect(ssid, 4223)
        except Exception as e:
            fatal_error("Failed to connect to ESP proxy")

        run_bricklet_tests(ipcon, result, qr_variant, qr_power, ssid, stage3)

        print("Waiting for NFC tags", end="")
        seen_tags = []
        while len(seen_tags) < 3:
            with urllib.request.urlopen("http://{}/nfc/seen_tags".format(ssid), timeout=1) as f:
                seen_tags = [x for x in stage3.get_nfc_tag_id() if any(y != 0 for y in x)]
            print("\rWaiting for NFC tags. {} seen".format(len(seen_tags)), end="")

        print("\r3 NFC tags seen. Configuring tags      ")
        req = urllib.request.Request("http://{}/nfc/config_update".format(ssid),
                                 data=json.dumps({"require_tag_to_start":False,
                                                  "require_tag_to_stop":False,
                                                  "authorized_tags": [{
                                                    "tag_name": "Tag 1",
                                                    "tag_type": seen_tags[0]["tag_type"],
                                                    "tag_id": seen_tags[0]["tag_id"]
                                                    }, {
                                                    "tag_name": "Tag 2",
                                                    "tag_type": seen_tags[1]["tag_type"],
                                                    "tag_id": seen_tags[1]["tag_id"]
                                                    }, {
                                                    "tag_name": "Tag 3",
                                                    "tag_type": seen_tags[2]["tag_type"],
                                                    "tag_id": seen_tags[2]["tag_id"]
                                                    }
                                                  ]}).encode("utf-8"),
                                 method='PUT',
                                 headers={"Content-Type": "application/json"})
        try:
            with urllib.request.urlopen(req, timeout=1) as f:
                f.read()
        except Exception as e:
            fatal_error("Failed to configure NFC tags!")
        result["nfc_tags_configured"] = True

    else:
        result["uid"] = None

        ipcon = IPConnection()
        ipcon.connect("localhost", 4223)

        run_bricklet_tests(ipcon, result, qr_variant, qr_power, None, stage3)
        print("Flashing EVSE")
        run(["python3", "comcu_flasher.py", result["evse_uid"], evse_path])
        result["evse_firmware"] = evse_path

    print("Checking if EVSE was tested...")
    if not exists_evse_test_report(result["evse_uid"]):
        if git_user is None:
            fatal_error("No test report found for EVSE {} and git username is unknown. Please pull the wallbox git.".format(result["evse_uid"]))
        print("No test report found. Checking for new test reports...")
        with ChangedDirectory(os.path.join("..", "..", "wallbox")):
            run(["su", git_user, "-c", "git pull"])
        if not exists_evse_test_report(result["evse_uid"]):
            fatal_error("No test report found for EVSE {}.".format(result["evse_uid"]))

    print("EVSE test report found")
    result["evse_test_report_found"] = True

    if qr_variant == "B":
        ssid = "warp2-" + result["evse_uid"]

    result["end"] = now()

    with open("{}_{}_report_stage_2.json".format(ssid, now().replace(":", "-")), "w") as f:
        json.dump(result, f, indent=4)

    print(green("Performing the electrical tests"))

    if qr_variant != "B":
        run(["su", git_user, "-c", "firefox --new-tab --url http://{}".format(ssid)])

    stage3.test_wallbox()

    print('Done!')

if __name__ == "__main__":
    try:
        led_wrap()
    except FatalError:
        input("Press return to exit. ")
        sys.exit(1)
    except Exception as e:
        traceback.print_exc()
        input("Press return to exit. ")
        sys.exit(1)
