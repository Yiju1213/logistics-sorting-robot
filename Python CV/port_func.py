from ctypes import sizeof
import numpy as np
from numpy import byte, int16, uint8, int8, uint16
import serial
import serial.tools.list_ports
from time import sleep
import struct 

def PortOpen():
    plist = list(serial.tools.list_ports.comports())
    if (len(plist) <= 0) :
        print('No serial port available!') 
    else:
        pSpecific = (list(plist[0]))[0]
        pSerial = serial.Serial(pSpecific,115200,timeout = 60)
        print('port %s open'%(pSpecific))
        return pSerial

def PackageRx(pSerial:serial):
    pSer = pSerial
    # 帧起始校验
    fs = (struct.unpack("B",pSer.read(size=1)))[0]
    if(fs == 0xFF):
        print("Starter Frame OK")
    else:
        print("Starter Error")
        return 
    # 帧长度获取
    dataByte = (struct.unpack("B",pSer.read(size=1)))[0]
    # print("Length of data bytes: ", dataByte)
    # 帧数据接收(获取的时候先用无符号接收，合并的时候再转为有符号)
    data = np.zeros(dataByte, uint8)
    for i in range (dataByte):
        data[i] = (struct.unpack("b",pSer.read(size=1)))[0]
        # print("data[" + str(i) + "]" + "=" + str(bin(data[i])))
    # 打印电机tarPos值，此时要转为int16
    RxTarPos = (int16((data[1]<<8)|(data[2])))
    # print("tarPos =  " , int16((data[1]<<8)|(data[2])) )
    # 帧结尾校验
    fe = (struct.unpack("B",pSer.read(size=1)))[0]
    if(fe == 0xAA):
        print("End Frame OK")
    else:
        print("End Frame Error")
    
    return RxTarPos

def PackageTx(pSerial:serial, data_byte:int, pData:np.ndarray):
    pSer = pSerial
    # 帧起始
    fs = 0xFF
    # 帧长度
    dataByte = data_byte
    # 帧数据 (uint8_t)
    # 传输的时候该用uint8，方便位运算
    data = np.zeros(dataByte, uint8)
    for i in range(dataByte):
        data[i] = pData[i]
        # print("data["+str(i)+"] = "+ str(bin(data[i])),"pData["+str(i)+"] = "+ str(bin(pData[i])))
    # 帧末尾
    fe = 0xAA
    # 打包
    pack = list(range(dataByte+2))
    pack[0] = struct.pack("BB", fs, dataByte)
    for i in range(dataByte):
        pack[i+1] = struct.pack("B",data[i]) # 同样用无符号形式传
    pack[dataByte+1] = struct.pack("B",fe)
    Pack = bytes("",'utf-8')
    for i in range(dataByte+2):
        Pack = Pack + pack[i]
    pSer.write(Pack)