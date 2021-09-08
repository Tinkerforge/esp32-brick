#!/usr/bin/python3 -u

import sys
import queue
import threading
import time
import traceback

from tinkerforge.ip_connection import IPConnection
from tinkerforge.device_factory import create_device
from tinkerforge.brick_master import BrickMaster
from tinkerforge.bricklet_industrial_dual_relay import BrickletIndustrialDualRelay
from tinkerforge.bricklet_industrial_quad_relay_v2 import BrickletIndustrialQuadRelayV2
from tinkerforge.bricklet_industrial_dual_ac_relay import BrickletIndustrialDualACRelay
from tinkerforge.bricklet_energy_monitor import BrickletEnergyMonitor

from provision_common.inventory import Inventory
from provision_common.provision_common import FatalError, fatal_error

STACK_0_MASTER_UID = '61SMKP'
ACTION_INTERVAL = 0.1 # seconds
MONOFLOP_DURATION = 1000 # milliseconds
SETTLE_DURATION = 0.25 # seconds

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

        self.ipcon.set_timeout(0.25)

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

    def setup(self):
        assert not self.prepared

        print('Connecting to brickd')

        try:
            self.ipcon.connect('tim2', 4223)
        except Exception as e:
            fatal_error('Could not connect to brickd: {0}'.format(e))

        print('Enumerating Bricklets')

        self.inventory.clear()
        self.inventory.update(timeout=0.5)

        self.devices = {}

        for brick_position in ['0', '1', '2']:
            full_position = '0' + brick_position

            if brick_position == '0':
                brick_entry = self.inventory.get_one(uid=STACK_0_MASTER_UID, connected_uid='0', position=brick_position)
            else:
                brick_entry = self.inventory.get_one(connected_uid=STACK_0_MASTER_UID, position=brick_position)

            if brick_entry == None:
                fatal_error('Missing Brick at postion {0}'.format(full_position))

            if EXPECTED_DEVICE_IDENTIFIERS[full_position] != brick_entry.device_identifier:
                fatal_error('Wrong Brick at postion {0}'.format(full_position))

            device = create_device(brick_entry.device_identifier, brick_entry.uid, self.ipcon)
            device.set_response_expected_all(True)

            self.devices[full_position] = device

            for bricklet_position in ['A', 'B', 'C', 'D']:
                full_position = '0' + brick_position + bricklet_position
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

    def idle(self):
        assert self.prepared

        print('Switching to idle target')

        self.connect_warp_power([])

        time.sleep(SETTLE_DURATION)

        self.connect_outlet(None)
        self.connect_voltage_monitor(False)
        self.change_cp_pe_state('A')
        self.change_meter_state('Type2-L1')

        time.sleep(SETTLE_DURATION)

    def power(self, outlet=None):
        assert self.prepared
        assert outlet in [None, 'Smart', 'Pro']

        if outlet == None:
            print('Switching to power target')
        else:
            print('Switching to power target for {} outlet'.format(outlet))

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
    # FIXME: targets: idle, power, test

    stage3 = Stage3()

    stage3.setup()
    stage3.power('Smart')

    input('Press return to exit ')

    stage3.teardown()

if __name__ == '__main__':
    try:
        main()
    except FatalError as e:
        input('Press return to exit ')
        sys.exit(1)
    except Exception as e:
        traceback.print_exc()
        input('Press return to exit ')
        sys.exit(1)
