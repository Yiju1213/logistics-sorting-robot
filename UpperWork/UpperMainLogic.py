from time import sleep
import cv2
import numpy as np
import threading, queue
from SerialPort import PackageTx
from ArucoDetection import mtx, dist
from LogisticStuffClass import DeliveryLocker, DeliveryPlace
from ProcessClass import LGS_Camera, LGS_Planner, LGS_SerialPort, UpperInstruction, DATABYTE


if __name__ == '__main__':
    
    # 快递柜对象(快递柜尺寸)
    lockersize = np.array([30, 40, 50], np.int32)
    LOCKERS = DeliveryLocker(lockerSize= lockersize, 
                             lockerArray= (2, 2))
    # 快递点对象（快递柜世界坐标）
    zerosOffset = np.array([30, 30, 0], np.int32)
    PLACE = DeliveryPlace(lockers= LOCKERS)
    PLACE.setZerosOffset(offset= zerosOffset)
    
    # 线程机制
    SerialQueue = queue.Queue()
    CamRunEvent = threading.Event()
    CamRunEvent.clear()
    CamMesQueue = queue.Queue()
    # 线程对象
    LGS_CAM = LGS_Camera(mtx= mtx, dist= dist, camCom= 0, singleOpenTime= 4, 
                         camMessageQueue= CamMesQueue, camRunEvent= CamRunEvent)
    LGS_SERIAL = LGS_SerialPort(serialQueue= SerialQueue, camRunEvent= CamRunEvent)
    LGS_PLANNER = LGS_Planner(deliveryPlace= PLACE, camMessageQueue= CamMesQueue)
    
    # 开启线程
    LGS_SERIAL.start()
    LGS_CAM.start()
    LGS_PLANNER.start()
    
    # 主线程模拟STM32发送信号
    while True:
        pack = PackageTx(None, DATABYTE, UpperInstruction.OPENCAM)
        SerialQueue.put(pack)
        
        sleep(4.5)
        LGS_PLANNER.getSortDict()
        sleep(0.5)
        input("press enter to continue")
        