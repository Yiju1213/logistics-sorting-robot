import numpy as np
from numpy import byte, int16, ndarray, uint8, int8, uint16
import serial
import serial.tools.list_ports
from time import sleep
import struct 


STD_FS = 0xFF
STD_FE = 0xAA



class SerialPort():
    def __init__(self, object:str) -> None:
        self.object = object
    def portOpen(self):
        pCom, pSerial = PortOpen()
        self.portCom = pCom
        self.handle = pSerial
    def PortClose(self):
        self.handle.close()
    
def PortOpen():
    plist = list(serial.tools.list_ports.comports())
    if (len(plist) <= 0) :
        print('No serial port available!') 
        return None
    else:
        pCom = (list(plist[0]))[0]
        pSerial = serial.Serial(pCom,115200,timeout = 60)
        print('port %s open'%(pCom))
        return pCom, pSerial



def PackageRx(pSerial:serial, pack=None):
    
    # 有端口
    if(pSerial is not None):
        pSer = pSerial
        # 帧起始校验
        fs = (struct.unpack("B",pSer.read(size=1)))[0]
        if(fs == STD_FS):
            pass
        else:
            print("Starter Error")
            return 
        # 帧长度获取
        dataByte = (struct.unpack("B",pSer.read(size=1)))[0]
        # 帧数据接收(获取的时候先用无符号接收，合并的时候再转为有符号)
        data = np.zeros(dataByte, uint8)
        for i in range (dataByte):
            data[i] = (struct.unpack("B",pSer.read(size=1)))[0]
        # 帧结尾校验
        fe = (struct.unpack("B",pSer.read(size=1)))[0]
        if(fe == STD_FE):
            pass
        else:
            print("End Frame Error")
            return
        # 返回数据
        return data
    # 无串口通道但有字节流包
    elif((pSerial is None) & (pack is not None)):
        unpacked = struct.unpack("BBhhhhhhhB", pack)
        data = np.array(list(unpacked[i] for i in range(2,9,1)), uint8)
        return data
    else:
        return None
        

def PackageTx(pSerial:serial, data_byte:int, pData:np.ndarray):
    # 创建字节流包(等待后续赋值)
    pack = list(range(data_byte+3))
    ##  pack[0]  |  pack[1]   |  pack[2] ~ pack[2+data_byte]  | pack[3_data_byte]  ##
    ##    FS     |  data_byte |  pData[0] ~ pData[data_byte]  |        FE          ##
    # 帧起始
    pack[0] = struct.pack("B", STD_FS)
    # 帧长度
    pack[1] = struct.pack("B", data_byte)
    # 帧数据 (uint8_t)
    
    for i in range(data_byte):
        pack[i+2] = struct.pack("h",pData[i]) 
    # 帧末尾
    pack[data_byte+2] = struct.pack("B",STD_FE)
    Pack = bytes("",'utf-8') 
    for i in range(data_byte+3):
        Pack = Pack + pack[i]
    # 有串口通道就传输
    if pSerial is not None:
        pSerial.write(Pack)
    # 无论是否有通道都返回字节流包
    return Pack