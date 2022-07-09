from ctypes import sizeof
import threading
import numpy as np
from numpy import byte, int16, size, uint8, int8
import serial
import serial.tools.list_ports
from time import sleep
import struct 
from SerialPort import PackageTx, PackageRx
import SerialPort
from _abandoned_2 import Stepper
from enum import Enum
from ProcessClass import Serial_Port
import queue

M42MMTOPULSE = 800
DATABYTE = 5
M1 = 1
M2 = 2

CamNeedOpen = threading.Event()

SerialQueue = queue.Queue()

Stepper1 = Stepper(M1)
Stepper2 = Stepper(M2)

Thread_Port = Serial_Port(queueHandle=SerialQueue)
Thread_Port.start()

# # 串口通道开启
# pSer = port_func.PortOpen()
# # 串口循环任务
while   True:
    # Packed = PackageTx(None, DATABYTE, UpperInstruction.SYNC)
    # dataArray = PackageRx(None, Packed)
    # print(dataArray)
    # LGSRobot_MessageClaim(dataArray)
#     # 准备发送数据
#     print("..................")
    tarPosInput1 = np.random.randint(-300, 300)
    print("randPos=",tarPosInput1)
    # print("tarPos =",tarPosInput1)
    tarVelInput1 = np.random.randint(-300, 300)
    print("randVel=",tarVelInput1)
#     # tarPosInput1 = int16(input("tarPosInput1 = "))
    # print(tarPosInput1)
    Stepper1.SetTarPos(tarPosInput1)
    Stepper1.SetTarVel(tarVelInput1)
    dataArray = Stepper1.GetStepperArray()
    packed = PackageTx(None, DATABYTE, dataArray)
    SerialQueue.put(item=packed, block=True)
#     tarPosInput2 = np.random.randint(0, 300)
#     # tarPosInput2 = int16(input("tarPosInput2 = "))
#     print(tarPosInput2)
#     Stepper2.SetTarPos(tarPosInput2)
    # print("Data Packaging...")
#     #print("data bytes:", DATABYTE)
#     #for i in range (DATABYTE):
#     #    print("data[" + str(i) + "]" + "=" + str(bin(Stepper1.TarPosArray[i])))
    # Pack = PackageTx(None, DATABYTE, Stepper1.TarPosArray)
#     sleep(0.001)
#     port_func.PackageTx(pSer, DATABYTE, Stepper2.TarPosArray)
    # print("Transmitting Data Package...")
    # print("............................")
    # print("Receiving Data Package......")
    
#     if(RxTarPos1 == tarPosInput1) :
#         Correct += 1
#     else:
#         Error += 1
    # PackageRx(None, pack= Pack)
#     print(RxTarPos2)
#     if(RxTarPos2 == tarPosInput2) :
#         Correct += 1
#     else:
#         Error += 1
#     CorrectRatio = (Correct / (Correct + Error))
#     print("Transmission Time = % d Correct Ratio = %f" % (int((Correct+Error)/2), CorrectRatio))
#     # print("Receiving Successfully")
    
    sleep(2)