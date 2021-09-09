#!/usr/bin/python3 -u

import sys
import queue
import threading
import time
import traceback

import cv2

from tinkerforge.ip_connection import IPConnection
from tinkerforge.device_factory import create_device
from tinkerforge.brick_master import BrickMaster
from tinkerforge.bricklet_industrial_dual_relay import BrickletIndustrialDualRelay
from tinkerforge.bricklet_industrial_quad_relay_v2 import BrickletIndustrialQuadRelayV2
from tinkerforge.bricklet_industrial_dual_ac_relay import BrickletIndustrialDualACRelay
from tinkerforge.bricklet_energy_monitor import BrickletEnergyMonitor
from tinkerforge.bricklet_servo_v2 import BrickletServoV2

from provision_common.inventory import Inventory
from provision_common.provision_common import FatalError, fatal_error

IPCON_HOST = 'localhost'

ACTION_INTERVAL = 0.1 # seconds

MONOFLOP_DURATION = 1000 # milliseconds

SETTLE_DURATION = 0.25 # seconds

STACK_MASTER_UIDS = {
    '0': '61SMKP',
    '1': '6jEhrp',
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
    '10': BrickMaster.DEVICE_IDENTIFIER,
    '10A': BrickletServoV2.DEVICE_IDENTIFIER,
}

class Stage3:
    def __init__(self):
        self.ipcon = IPConnection()
        self.inventory = Inventory(self.ipcon)
        self.devices = {} # by position path
        self.prepared = False
        self.action_stop_queue = None
        self.action_enabled_ref = None
        self.action_thread = None

        self.ipcon.set_timeout(0.5)

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
                    function(self.devices[position])
                except Exception as e:
                    fatal_error('Could not execute function for device at position {0}: {1}'.format(position, e))

    def connect_warp_power(self, phases):
        assert set(phases).issubset({'L1', 'L2', 'L3'}), phases

        if len(phases) == 0:
            self.action_stop_queue.put(('00D', lambda device: device.set_value(False, False)))
            self.action_stop_queue.put(('01A', lambda device: device.set_value(False, False)))
        else:
            assert 'L1' in phases, phases

            self.action_stop_queue.put(('00D', lambda device: [device.set_monoflop(0, True, MONOFLOP_DURATION), device.set_monoflop(1, True, MONOFLOP_DURATION)]))

            if 'L2' in phases and 'L3' in phases:
                self.action_stop_queue.put(('01A', lambda device: [device.set_monoflop(0, True, MONOFLOP_DURATION), device.set_monoflop(1, True, MONOFLOP_DURATION)]))
            elif 'L2' in phases:
                self.action_stop_queue.put(('01A', lambda device: [device.set_selected_value(0, False), device.set_monoflop(1, True, MONOFLOP_DURATION)]))
            elif 'L3' in phases:
                self.action_stop_queue.put(('01A', lambda device: [device.set_monoflop(0, True, MONOFLOP_DURATION), device.set_selected_value(1, False)]))

    def connect_outlet(self, outlet):
        assert outlet in [None, 'Smart', 'Pro']

        if outlet == None:
            self.action_stop_queue.put(('02A', lambda device: device.set_value(False, False)))
        elif outlet == 'Smart':
            self.action_stop_queue.put(('02A', lambda device: [device.set_monoflop(0, True, MONOFLOP_DURATION), device.set_selected_value(1, False)]))
        elif outlet == 'Pro':
            self.action_stop_queue.put(('02A', lambda device: [device.set_selected_value(0, False), device.set_monoflop(1, True, MONOFLOP_DURATION)]))
        else:
            assert False, outlet

    def connect_voltage_monitor(self, connect):
        if connect:
            self.action_stop_queue.put(('01B', lambda device: [device.set_monoflop(0, True, MONOFLOP_DURATION), device.set_monoflop(1, True, MONOFLOP_DURATION)]))
            self.action_stop_queue.put(('01C', lambda device: [device.set_monoflop(0, True, MONOFLOP_DURATION), device.set_monoflop(1, True, MONOFLOP_DURATION)]))
        else:
            self.action_stop_queue.put(('01B', lambda device: device.set_value(False, False)))
            self.action_stop_queue.put(('01C', lambda device: device.set_value(False, False)))

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

        self.action_stop_queue.put(('01D', lambda device: device.set_value(value)))

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

        self.action_stop_queue.put(('00A', lambda device: device.set_value(*value[0])))
        self.action_stop_queue.put(('00B', lambda device: device.set_value(*value[1])))
        self.action_stop_queue.put(('00C', lambda device: device.set_value(*value[2])))

    def set_servo_position(self, servo, channel, position):
        servo.set_position(channel, position)

        current_position = servo.get_current_position(channel)

        # FIXME: add timeout?
        while current_position != position:
            time.sleep(0.1)

            current_position = servo.get_current_position(channel)

    def trigger_meter_run(self):
        servo = self.devices['10A']

        try:
            if not servo.get_enabled(0):
                servo.set_enable(0, True)

            self.set_servo_position(servo, 0, 4000)
            self.set_servo_position(servo, 0, 6800)
            time.sleep(0.1)
            self.set_servo_position(servo, 0, 4000)
        except Exception as e:
            fatal_error('Could not press meter run button: {0}'.format(e))

    def trigger_meter_back(self):
        servo = self.devices['10A']

        try:
            if not servo.get_enabled(1):
                servo.set_enable(1, True)

            self.set_servo_position(servo, 1, 3000)
            self.set_servo_position(servo, 1, 5500)
            time.sleep(0.1)
            self.set_servo_position(servo, 1, 3000)
        except Exception as e:
            fatal_error('Could not press meter back button: {0}'.format(e))

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

        for stack_position, positions in [('0', {'0': ['A', 'B', 'C', 'D'], '1': ['A', 'B', 'C', 'D'], '2': ['A', 'B', 'C', 'D']}),
                                          ('1', {'0': ['A']})]:
            for brick_position, bricklet_positions in positions.items():
                full_position = stack_position + brick_position

                if brick_position == '0':
                    brick_entry = self.inventory.get_one(uid=STACK_MASTER_UIDS[stack_position], connected_uid='0', position=brick_position)
                else:
                    brick_entry = self.inventory.get_one(connected_uid=stack_master_uid, position=brick_position)

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

                    device = create_device(bricklet_entry.device_identifier, bricklet_entry.uid, self.ipcon)
                    device.set_response_expected_all(True)

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
        self.change_cp_pe_state('A')
        self.change_meter_state('Type2-L1')

        time.sleep(SETTLE_DURATION)

        self.connect_warp_power(['L1'])

        time.sleep(SETTLE_DURATION)

def main():
    stage3 = Stage3()

    stage3.setup()

    #stage3.power_on('Smart')
    #stage3.trigger_meter_run()
    #stage3.trigger_meter_back()
    print(stage3.read_meter_qr_code(20))

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
