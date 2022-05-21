from ctypes import sizeof
import numpy as np
from numpy import byte, int16, size, uint8, int8
import serial
import serial.tools.list_ports
from time import sleep
import struct 

import port_func
from StepperPlanning import Stepper
M42MMTOPULSE = 800
DATABYTE = 3
M1 = 1
M2 = 2
Stepper1 = Stepper(M1)
Stepper2 = Stepper(M2)
Correct = 0
Error   = 0
# 串口通道开启
pSer = port_func.PortOpen()
pSer.write
# 串口循环任务
while   1:
    # 准备发送数据
    print("..................")
    tarPosInput1 = np.random.randint(0, 350)
    # tarPosInput1 = int16(input("tarPosInput1 = "))
    print(tarPosInput1)
    Stepper1.SetTarPos(tarPosInput1)
    tarPosInput2 = np.random.randint(0, 350)
    # tarPosInput2 = int16(input("tarPosInput2 = "))
    print(tarPosInput2)
    Stepper2.SetTarPos(tarPosInput2)
    print("Data Packaging...")
    #print("data bytes:", DATABYTE)
    #for i in range (DATABYTE):
    #    print("data[" + str(i) + "]" + "=" + str(bin(Stepper1.TarPosArray[i])))
    port_func.PackageTx(pSer, DATABYTE, Stepper1.TarPosArray)
    sleep(0.001)
    port_func.PackageTx(pSer, DATABYTE, Stepper2.TarPosArray)
    print("Transmitting Data Package...")
    print("............................")
    print("Receiving Data Package......")
    RxTarPos1 = port_func.PackageRx(pSer)
    print(RxTarPos1)
    if(RxTarPos1 == tarPosInput1) :
        Correct += 1
    else:
        Error += 1
    RxTarPos2 = port_func.PackageRx(pSer)
    print(RxTarPos2)
    if(RxTarPos2 == tarPosInput2) :
        Correct += 1
    else:
        Error += 1
    CorrectRatio = (Correct / (Correct + Error))
    print("Transmission Time = % d Correct Ratio = %f" % (int((Correct+Error)/2), CorrectRatio))
    # print("Receiving Successfully")
    input("press any to continue...")
    sleep(0.1)