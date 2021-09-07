#!/usr/bin/python3 -u

from tinkerforge.ip_connection import IPConnection
from tinkerforge.device_factory import create_device
from tinkerforge.brick_master import BrickMaster
from tinkerforge.bricklet_industrial_dual_relay import BrickletIndustrialDualRelay
from tinkerforge.bricklet_industrial_quad_relay_v2 import BrickletIndustrialQuadRelayV2
from tinkerforge.bricklet_industrial_dual_ac_relay import BrickletIndustrialDualACRelay
from tinkerforge.bricklet_energy_monitor import BrickletEnergyMonitor

from provision_common.inventory import Inventory
from provision_common.provision_common import fatal_error

EXPECTED_DEVICE_IDENTIFIERS = {
    '0': BrickMaster.DEVICE_IDENTIFIER,
    '0A': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '0B': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '0C': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '0D': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '1': BrickMaster.DEVICE_IDENTIFIER,
    '1A': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '1B': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '1C': BrickletIndustrialDualRelay.DEVICE_IDENTIFIER,
    '1D': BrickletIndustrialQuadRelayV2.DEVICE_IDENTIFIER,
    '2': BrickMaster.DEVICE_IDENTIFIER,
    '2A': BrickletIndustrialDualACRelay.DEVICE_IDENTIFIER,
    '2B': BrickletEnergyMonitor.DEVICE_IDENTIFIER,
    '2C': BrickletEnergyMonitor.DEVICE_IDENTIFIER,
    '2D': BrickletEnergyMonitor.DEVICE_IDENTIFIER,
}

class Stage3:
    def __init__(self):
        self.ipcon = IPConnection()
        self.inventory = Inventory(self.ipcon)
        self.devices = {} # by position path

    def connect_warp_power(self, connect):
        try:
            self.devices['0D'].set_value(connect, connect)
            self.devices['1A'].set_value(connect, connect)
        except Exception as e:
            fatal_error('Could not {0}connect WARP power: {1}'.format('' if connect else 'dis', e))

    def connect_smart_outlet(self, connect):
        try:
            self.devices['2A'].set_selected_value(0, connect)
        except Exception as e:
            fatal_error('Could not {0}connect Smart outlet: {1}'.format('' if connect else 'dis', e))

    def connect_pro_outlet(self, connect):
        try:
            self.devices['2A'].set_selected_value(1, connect)
        except Exception as e:
            fatal_error('Could not {0}connect Pro outlet: {1}'.format('' if connect else 'dis', e))

    def connect_voltage_monitor(self, connect):
        try:
            self.devices['1B'].set_value(connect, connect)
            self.devices['1C'].set_value(connect, connect)
        except Exception as e:
            fatal_error('Could not {0}connect voltage monitor: {1}'.format('' if connect else 'dis', e))

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

        try:
            self.devices['1D'].set_value(value)
        except Exception as e:
            fatal_error('Could not change CP-PE state to {0}: {1}'.format(state, e))

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

        try:
            self.devices['0A'].set_value(*value[0])
            self.devices['0B'].set_value(*value[1])
            self.devices['0C'].set_value(*value[2])
        except Exception as e:
            fatal_error('Could not change meter state to {0}: {1}'.format(state, e))

    def run(self):
        print('Connecting to brickd')

        try:
            self.ipcon.connect('tim2', 4223)
        except Exception as e:
            fatal_error('Could not connect to brickd: {0}'.format(e))

        print('Enumerating Bricklets')

        self.inventory.update(timeout=0.5)

        for brick_position in ['0', '1', '2']:
            brick_entry = self.inventory.get_one(position=brick_position)

            if brick_entry == None:
                fatal_error('Missing Brick at postion {0}'.format(brick_position))

            if EXPECTED_DEVICE_IDENTIFIERS[brick_position] != brick_entry.device_identifier:
                fatal_error('Wrong Brick at postion {0}'.format(brick_position))

            device = create_device(brick_entry.device_identifier, brick_entry.uid, self.ipcon)
            device.set_response_expected_all(True)

            self.devices[brick_position] = device

            for bricklet_position in ['A', 'B', 'C', 'D']:
                bricklet_entry = self.inventory.get_one(connected_uid=brick_entry.uid, position=bricklet_position)

                if bricklet_entry == None:
                    fatal_error('Missing Bricklet at postion {0}'.format(brick_position + bricklet_position))

                if EXPECTED_DEVICE_IDENTIFIERS[brick_position + bricklet_position] != bricklet_entry.device_identifier:
                    fatal_error('Wrong Bricklet at postion {0}'.format(brick_position + bricklet_position))

                device = create_device(bricklet_entry.device_identifier, bricklet_entry.uid, self.ipcon)
                device.set_response_expected_all(True)

                self.devices[brick_position + bricklet_position] = device

        print('Switching to idle state')

        self.connect_warp_power(False)
        self.connect_smart_outlet(False)
        self.connect_pro_outlet(False)
        self.connect_voltage_monitor(False)
        self.change_cp_pe_state('A')
        self.change_meter_state('Type2-L1')

def main():
    Stage3().run()

if __name__ == '__main__':
    main()
