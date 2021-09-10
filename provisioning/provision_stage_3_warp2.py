#!/usr/bin/python3 -u

import sys
import queue
import threading
import time
import traceback

import cv2 # sudo pip3 install openvc-python

from tinkerforge.ip_connection import IPConnection
from tinkerforge.device_factory import create_device
from tinkerforge.brick_master import BrickMaster
from tinkerforge.bricklet_industrial_dual_relay import BrickletIndustrialDualRelay
from tinkerforge.bricklet_industrial_quad_relay_v2 import BrickletIndustrialQuadRelayV2
from tinkerforge.bricklet_industrial_dual_ac_relay import BrickletIndustrialDualACRelay
from tinkerforge.bricklet_energy_monitor import BrickletEnergyMonitor
from tinkerforge.bricklet_servo_v2 import BrickletServoV2
from tinkerforge.bricklet_led_strip_v2 import BrickletLEDStripV2
from tinkerforge.bricklet_color_v2 import BrickletColorV2

from provision_common.bricklet_nfc import BrickletNFC
from provision_common.inventory import Inventory
from provision_common.provision_common import FatalError, fatal_error

IPCON_HOST = 'localhost'
IPCON_TIMEOUT = 1.5 # seconds

ACTION_INTERVAL = 0.2 # seconds

MONOFLOP_DURATION = 2000 # milliseconds

SETTLE_DURATION = 0.25 # seconds

STACK_MASTER_UIDS = {
    '0': '61SMKP',
    '1': '6jEhrp',
    '2': '6DY5kB',
}

EXPECTED_DEVICE_IDENTIFIERS = {
    '00': BrickMaster.DEVICE_IDENTIFIER,
    '00A': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '00B': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '00C': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '00D': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '01': BrickMaster.DEVICE_IDENTIFIER,
    '01A': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '01B': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '01C': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '01D': BrickletIndustrialQuadRelayV2.DEVICE_IDENTIFIER,
    '02': BrickMaster.DEVICE_IDENTIFIER,
    '02A': BrickletIndustrialDualACRelay.DEVICE_IDENTIFIER,
    '02B': BrickletEnergyMonitor.DEVICE_IDENTIFIER,
    '02C': BrickletEnergyMonitor.DEVICE_IDENTIFIER,
    '02D': BrickletEnergyMonitor.DEVICE_IDENTIFIER,
    '03': BrickMaster.DEVICE_IDENTIFIER,
    '03A': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '10': BrickMaster.DEVICE_IDENTIFIER,
    '10A': BrickletServoV2.DEVICE_IDENTIFIER,
    '20': BrickMaster.DEVICE_IDENTIFIER,
    '20A': BrickletLEDStripV2.DEVICE_IDENTIFIER,
    '20B': BrickletNFC.DEVICE_IDENTIFIER,
    '20C': BrickletServoV2.DEVICE_IDENTIFIER,
    '20D': BrickletColorV2.DEVICE_IDENTIFIER,
}

class Stage3:
    def __init__(self, is_front_panel_button_pressed_function, get_iec_state_function, reset_dc_fault_function):
        self.is_front_panel_button_pressed_function = is_front_panel_button_pressed_function
        self.get_iec_state_function = get_iec_state_function
        self.reset_dc_fault_function = reset_dc_fault_function
        self.ipcon = IPConnection()
        self.inventory = Inventory(self.ipcon)
        self.devices = {} # by position path
        self.prepared = False
        self.action_stop_queue = None
        self.action_enabled_ref = None
        self.action_thread = None

        self.ipcon.set_timeout(IPCON_TIMEOUT)

    # internal
    def action_loop(self, stop_queue, enabled_ref):
        timestamp = -ACTION_INTERVAL
        actions = {} # by position path

        while enabled_ref[0]:
            elapsed = time.monotonic() - timestamp
            remaining = max(ACTION_INTERVAL - elapsed, 0)

            try:
                action = stop_queue.get(timeout=remaining)
            except queue.Empty:
                action = False

            if not enabled_ref[0] or action == None:
                break

            timestamp = time.monotonic()

            if action != False:
                position, function = action
                actions[position] = function

            for position, function in actions.items():
                try:
                    function(self.devices[position[0]])
                except Exception as e:
                    fatal_error('Could not execute function {0} for device at position {1}: {2}'.format(position[1], position[0], e))

    # internal
    def connect_warp_power(self, phases):
        assert set(phases).issubset({'L1', 'L2', 'L3'}), phases

        if len(phases) == 0:
            self.action_stop_queue.put((('00D', 0), lambda device: device.set_value(False, False)))
            self.action_stop_queue.put((('01A', 0), lambda device: device.set_value(False, False)))
        else:
            assert 'L1' in phases, phases

            self.action_stop_queue.put((('00D', 0), lambda device: [device.set_monoflop(0, True, MONOFLOP_DURATION), device.set_monoflop(1, True, MONOFLOP_DURATION)]))

            if 'L2' in phases and 'L3' in phases:
                self.action_stop_queue.put((('01A', 0), lambda device: [device.set_monoflop(0, True, MONOFLOP_DURATION), device.set_monoflop(1, True, MONOFLOP_DURATION)]))
            elif 'L2' in phases:
                self.action_stop_queue.put((('01A', 0), lambda device: [device.set_selected_value(0, False), device.set_monoflop(1, True, MONOFLOP_DURATION)]))
            elif 'L3' in phases:
                self.action_stop_queue.put((('01A', 0), lambda device: [device.set_monoflop(0, True, MONOFLOP_DURATION), device.set_selected_value(1, False)]))

    # internal
    def connect_outlet(self, outlet):
        assert outlet in [None, 'Smart', 'Pro']

        if outlet == None:
            self.action_stop_queue.put((('02A', 0), lambda device: device.set_value(False, False)))
        elif outlet == 'Smart':
            self.action_stop_queue.put((('02A', 0), lambda device: [device.set_monoflop(0, True, MONOFLOP_DURATION), device.set_selected_value(1, False)]))
        elif outlet == 'Pro':
            self.action_stop_queue.put((('02A', 0), lambda device: [device.set_selected_value(0, False), device.set_monoflop(1, True, MONOFLOP_DURATION)]))
        else:
            assert False, outlet

    # internal
    def connect_voltage_monitor(self, connect):
        if connect:
            self.action_stop_queue.put((('01B', 0), lambda device: [device.set_monoflop(0, True, MONOFLOP_DURATION), device.set_monoflop(1, True, MONOFLOP_DURATION)]))
            self.action_stop_queue.put((('01C', 0), lambda device: [device.set_monoflop(0, True, MONOFLOP_DURATION), device.set_monoflop(1, True, MONOFLOP_DURATION)]))
        else:
            self.action_stop_queue.put((('01B', 0), lambda device: device.set_value(False, False)))
            self.action_stop_queue.put((('01C', 0), lambda device: device.set_value(False, False)))

    # internal
    def connect_front_panel(self, connect):
        if connect:
            self.action_stop_queue.put((('03A', 0), lambda device: device.set_monoflop(0, True, MONOFLOP_DURATION)))
        else:
            self.action_stop_queue.put((('03A', 0), lambda device: device.set_selected_value(0, False)))

    # internal
    def connect_type2_pe(self, connect):
        if connect:
            self.action_stop_queue.put((('03A', 1), lambda device: device.set_selected_value(1, False)))
        else:
            self.action_stop_queue.put((('03A', 1), lambda device: device.set_monoflop(1, True, MONOFLOP_DURATION)))

    # internal
    def change_cp_pe_state(self, state):
        if state == 'A':
            value = [False, False, False, False]
        elif state == 'B':
            value = [False, True,  False, False]
        elif state == 'C':
            value = [False, True,  True,  False]
        elif state == 'D':
            value = [False, True,  False, True]
        else:
            assert False, state

        self.action_stop_queue.put((('01D', 0), lambda device: device.set_value(value)))

    # internal
    def change_meter_state(self, state):
        if state == 'Type2-L1':
            value = [(True,  True),  (False, False), (False, False)]
        elif state == 'Type2-L2':
            value = [(True,  True),  (True,  False), (False, False)]
        elif state == 'Type2-L3':
            value = [(True,  True),  (True,  False), (True,  False)]
        elif state == 'WARP-L1':
            value = [(False, False), (False, False), (False, False)]
        elif state == 'WARP-L2':
            value = [(False, False), (False, True),  (False, False)]
        elif state == 'WARP-L3':
            value = [(False, False), (False, True),  (False, True)]
        else:
            assert False, state

        self.action_stop_queue.put((('00A', 0), lambda device: device.set_value(*value[0])))
        self.action_stop_queue.put((('00B', 0), lambda device: device.set_value(*value[1])))
        self.action_stop_queue.put((('00C', 0), lambda device: device.set_value(*value[2])))

    # internal
    def read_voltage_monitor(self, phase):
        if phase == 'L1':
            position = '02B'
        elif phase == 'L2':
            position = '02C'
        elif phase == 'L3':
            position = '02D'
        else:
            assert False, phase

        try:
            data = self.devices[position].get_energy_data()
        except Exception as e:
            fatal_error('Could not read voltage monitor: {0}'.format(e))

        return data[0] / 100

    # internal
    def set_servo_position(self, servo, channel, position):
        servo.set_position(channel, position)

        current_position = servo.get_current_position(channel)

        # FIXME: add timeout?
        while current_position != position:
            time.sleep(0.1)

            current_position = servo.get_current_position(channel)

    # internal
    def click_meter_run_button(self):
        servo = self.devices['10A']
        channel = 0

        try:
            self.set_servo_position(servo, channel, 4000)

            if not servo.get_enabled(channel):
                servo.set_enable(channel, True)

            self.set_servo_position(servo, channel, 6800)
            time.sleep(0.1)
            self.set_servo_position(servo, channel, 4000)
            time.sleep(0.1)
        except Exception as e:
            fatal_error('Could not click meter run button: {0}'.format(e))

    # internal
    def click_meter_back_button(self):
        servo = self.devices['10A']
        channel = 1

        try:
            self.set_servo_position(servo, channel, 3000)

            if not servo.get_enabled(channel):
                servo.set_enable(channel, True)

            self.set_servo_position(servo, channel, 5500)
            time.sleep(0.1)
            self.set_servo_position(servo, channel, 3000)
            time.sleep(0.1)
        except Exception as e:
            fatal_error('Could not click meter back button: {0}'.format(e))

    # internal
    def read_meter_qr_code(self, timeout=None):
        text = ''
        timestamp = time.monotonic()

        try:
            capture = cv2.VideoCapture(0)
            decoder = cv2.QRCodeDetector()

            while len(text) == 0:
                _, frame = capture.read()
                text, _, _ = decoder.detectAndDecode(frame)

                if timeout == None or timestamp + timeout < time.monotonic():
                    break

                time.sleep(0.1)

            capture.release()
        except Exception as e:
            fatal_error('Could not read QR code: {0}'.format(e))

        return text

    # internal
    def is_front_panel_led_on(self):
        try:
            color = self.devices['20D'].get_color()
        except Exception as e:
            fatal_error('Could not read front panel LED color: {0}'.format(e))

        return color[2] - color[0] > 1000 # FIXME: adjust threshold

    def setup(self):
        assert not self.prepared

        print('Connecting to brickd')

        try:
            self.ipcon.connect(IPCON_HOST, 4223)
        except Exception as e:
            fatal_error('Could not connect to brickd: {0}'.format(e))

        print('Enumerating Bricklets')

        self.inventory.clear()
        self.inventory.update(timeout=0.5)

        self.devices = {}

        for stack_position, positions in [('0', {'0': ['A', 'B', 'C', 'D'], '1': ['A', 'B', 'C', 'D'], '2': ['A', 'B', 'C', 'D'], '3': ['A']}),
                                          ('1', {'0': ['A']}),
                                          ('2', {'0': ['A', 'B', 'C', 'D']})]:
            for brick_position, bricklet_positions in positions.items():
                full_position = stack_position + brick_position

                if brick_position == '0':
                    brick_entry = self.inventory.get_one(uid=STACK_MASTER_UIDS[stack_position], connected_uid='0', position=brick_position)
                else:
                    brick_entry = self.inventory.get_one(connected_uid=STACK_MASTER_UIDS[stack_position], position=brick_position)

                if brick_entry == None:
                    fatal_error('Missing Brick at postion {0}'.format(full_position))

                if EXPECTED_DEVICE_IDENTIFIERS[full_position] != brick_entry.device_identifier:
                    fatal_error('Wrong Brick at postion {0}'.format(full_position))

                device = create_device(brick_entry.device_identifier, brick_entry.uid, self.ipcon)
                device.set_response_expected_all(True)

                self.devices[full_position] = device

                for bricklet_position in bricklet_positions:
                    full_position = stack_position + brick_position + bricklet_position
                    bricklet_entry = self.inventory.get_one(connected_uid=brick_entry.uid, position=bricklet_position)

                    if bricklet_entry == None:
                        fatal_error('Missing Bricklet at postion {0}'.format(full_position))

                    if EXPECTED_DEVICE_IDENTIFIERS[full_position] != bricklet_entry.device_identifier:
                        fatal_error('Wrong Bricklet at postion {0}'.format(full_position))

                    if bricklet_entry.device_identifier == BrickletNFC.DEVICE_IDENTIFIER:
                        device = BrickletNFC(bricklet_entry.uid, self.ipcon)
                    else:
                        device = create_device(bricklet_entry.device_identifier, bricklet_entry.uid, self.ipcon)

                    device.set_response_expected_all(True)

                    if bricklet_entry.device_identifier == BrickletColorV2.DEVICE_IDENTIFIER:
                        try:
                            device.set_status_led_config(BrickletColorV2.STATUS_LED_CONFIG_OFF)
                        except Exception as e:
                            fatal_error('Could not disable status LED for Color Bricklet 2.0 at postion {0}: {1}'.format(full_position, e))
                    elif bricklet_entry.device_identifier == BrickletNFC.DEVICE_IDENTIFIER:
                        try:
                            device.set_mode(BrickletNFC.MODE_SIMPLE)
                        except Exception as e:
                            fatal_error('Could not set simple mode for NFC Bricklet at postion {0}: {1}'.format(full_position, e))

                    self.devices[full_position] = device

        self.action_stop_queue = queue.Queue()
        self.action_enabled_ref = [True]
        self.action_thread = threading.Thread(target=self.action_loop, args=(self.action_stop_queue, self.action_enabled_ref), daemon=True)
        self.action_thread.start()

        self.prepared = True

    def teardown(self):
        assert self.prepared

        self.prepared = False

        self.action_enabled_ref[0] = False
        self.action_stop_queue.put(None)
        self.action_thread.join()

        self.action_stop_queue = None
        self.action_enabled_ref = None
        self.action_thread = None

    def power_off(self):
        assert self.prepared

        print('Switching power off')

        self.connect_warp_power([])

        time.sleep(SETTLE_DURATION)

        self.connect_outlet(None)
        self.connect_voltage_monitor(False)
        self.connect_front_panel(False)
        self.connect_type2_pe(True)
        self.change_cp_pe_state('A')
        self.change_meter_state('Type2-L1')

        time.sleep(SETTLE_DURATION)

    def power_on(self, outlet=None):
        assert self.prepared
        assert outlet in [None, 'Smart', 'Pro']

        if outlet == None:
            print('Switching power on')
        else:
            print('Switching power on for {} outlet'.format(outlet))

        self.connect_warp_power([])

        time.sleep(SETTLE_DURATION)

        self.connect_outlet(outlet)
        self.connect_voltage_monitor(False)
        self.connect_front_panel(False)
        self.connect_type2_pe(True)
        self.change_cp_pe_state('A')
        self.change_meter_state('Type2-L1')

        time.sleep(SETTLE_DURATION)

        self.connect_warp_power(['L1'])

        time.sleep(SETTLE_DURATION)

    def test_front_panel_button(self):
        assert self.is_front_panel_button_pressed_function != None

        servo = self.devices['20C']
        channel = 0

        try:
            button_before = self.is_front_panel_button_pressed_function()
            led_before = self.is_front_panel_led_on()

            if button_before:
                fatal_error('Front panel button is already pressed before test')

            if not led_before:
                fatal_error('Front panel LED is not on before test')

            self.set_servo_position(servo, channel, -3000)

            if not servo.get_enabled(channel):
                servo.set_enable(channel, True)

            self.set_servo_position(servo, channel, 2100)
            time.sleep(0.1)

            button_pressed = self.is_front_panel_button_pressed_function()
            led_pressed = self.is_front_panel_led_on()

            if not button_before:
                fatal_error('Front panel button is not pressed during test')

            if led_before:
                fatal_error('Front panel LED is still on during test')

            self.set_servo_position(servo, channel, -3000)
            time.sleep(0.1)

            button_after = self.is_front_panel_button_pressed_function()
            led_after = self.is_front_panel_led_on()

            if button_after:
                fatal_error('Front panel button is still pressed after test')

            if not led_after:
                fatal_error('Front panel LED is not on after test')
        except Exception as e:
            fatal_error('Could not test front panel button: {0}'.format(e))

    def get_nfc_tag_id(self, index):
        return self.devices['20B'].simple_get_tag_id(index)

    def test_wallbox(self):
        pass

def main():
    stage3 = Stage3(is_front_panel_button_pressed_function=lambda: False, get_iec_state_function=lambda: 'A', reset_dc_fault_function=lambda: None)

    stage3.setup()

    #stage3.power_on('Smart')
    #stage3.click_meter_run_button()
    #stage3.click_meter_back_button()
    #print(stage3.read_meter_qr_code(20))
    #stage3.connect_front_panel(True)
    #stage3.connect_type2_pe(False)
    #print(stage3.read_voltage_monitor('L3'))
    #stage3.test_front_panel_button()
    #print(stage3.get_nfc_tag_id(0))
    #print(stage3.test_wallbox())

    input('Press return to exit ')

    stage3.teardown()

if __name__ == '__main__':
    try:
        main()
    except FatalError:
        input('Press return to exit ')
        sys.exit(1)
    except Exception as e:
        traceback.print_exc()
        input('Press return to exit ')
        sys.exit(1)
