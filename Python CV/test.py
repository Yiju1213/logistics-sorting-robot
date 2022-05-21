from ctypes import sizeof
import numpy as np
from numpy import byte, uint8
import serial
import serial.tools.list_ports
from time import sleep
import struct 

import port_func
import StepperPlanning
M1 = 1

Stepper1 = StepperPlanning.Stepper(M1)
Stepper1.SetTarPos(416)
print(Stepper1.name, Stepper1.TarPos, Stepper1.TarPosByteL, bin(Stepper1.TarPosByteL))
print(Stepper1.TarPosArray)
print((Stepper1.TarPosByteH << 8)|(Stepper1.TarPosByteL))