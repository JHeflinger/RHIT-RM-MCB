# -*- coding: utf-8 -*-
# Copyright (c) 2013-2016, Niklas Hauser
# Copyright (c)      2016, Fabian Greif
# All rights reserved.

class Peripheral():
    """ Peripheral
    Represents the names and masks of the peripherals control and data registers
    and methods for smart comparison.
    """

    def __init__(self, name=None, registers=None):
        self.name = name
        if registers == None:
            registers = []
        self.registers = registers

    def addRegister(self, register):
        self.registers.append(register)

    def getComparisonDict(self, other):
        pass

    def getComparisonPeripheral(self, other):
        assert isinstance(other, Peripheral)

        comparision_dict = {'common_keys': [], 'different_keys': []}

        common = Peripheral()
        if self.name == other.name:
            common.name = self.name
        self_delta = Peripheral(self.name)
        other_delta = Peripheral(other.name)

        # compare registers
        self_regs = list(self.registers)
        other_regs = list(other.registers)

        common.registers = list(set(self_regs).intersection(other_regs))
        self_delta.registers = list(set(self_regs).difference(other_regs))
        other_delta.registers = list(set(other_regs).difference(self_regs))

        comparision_dict['common'] = common
        comparision_dict['self_delta'] = self_delta
        comparision_dict['other_delta'] = other_delta

        return comparision_dict

    def isEmpty(self):
        return len(self.registers) == 0

    def __eq__(self, other):
        if isinstance(other, Peripheral):
            comp = self.getComparisonPeripheral(other)
            if comp['self_delta'].isEmpty() and comp['other_delta'].isEmpty():
                return True
            return False
        return NotImplemented

    def __hash__(self):
        return hash(self.name + str(self.registers))

    def __ne__(self, other):
        result = self.__eq__(other)
        if result is NotImplemented:
            return result
        return not result

    def __repr__(self):
        return "Peripheral(" + str(self.name) + ")"

    def __str__(self):
        s = "\n Peripheral(\n\t{'name': '" + str(self.name) + "',\n"
        s += "\t'registers': ["
        st = ""
        for reg in self.registers:
            st += str(reg)
        st = st.replace("\n", "\n\t")
        return s + st + " ]})"
