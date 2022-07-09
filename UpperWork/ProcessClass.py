import threading  
import time
import numpy as np
import cv2
import queue
from SerialPort import PackageRx, PackageTx, PortOpen
from ArucoDetection import ArucoDetectionWork
from LogisticStuffClass import DeliveryPlace, DeliveryLocker
from pprint import pprint

##################### LGS串口部分 #####################

DATABYTE = 7
CurPosValue = 0

# 约定的下位机交互指令
class UpperInstruction():
    # 为序列值指定value值
    SYNC        = np.array([0x1,0,0,0,0,0,0])
    OPENCAM     = np.array([0x2,0,0,0,0,0,0])
    CAM_OPENED  = np.array([0x3,0,0,0,0,0,0])
    GETPOS      = np.array([0x4,0,0,0,0,0,0])

# 串口信息响应函数
def LGSRobot_MessageRespond(pData: np.ndarray, camRunEvent:threading.Event):
    # 对于已经约定好的通信结构进行拆分
    # 对于下位机指令只有最末位有用
    if   all(pData == UpperInstruction.SYNC):
        print("Receive SYNC Request")
        print("Sending SYNC Signal...")
        PackageTx(None, DATABYTE, UpperInstruction.SYNC)
    elif all(pData == UpperInstruction.OPENCAM):
        # print("Receive CAM Open Request")
        print("-----------------")
        print("Opening CAM ...")
        # 设置相机运行指令
        camRunEvent.set()
        PackageTx(None, DATABYTE, UpperInstruction.CAM_OPENED)
    elif all(pData == UpperInstruction.GETPOS):
        print("Receive Control Messages Request")
        print("Sending Positions for Control...")
        global CurPosValue
        
        # 串口发送消息
        ...
        # 打印消息
        
    else:
        print("Request Can't Explain")
        # 异常响应
        ...

# 面向LGS机器人的串口工作线程
class LGS_SerialPort(threading.Thread):
    def __init__(self, serialQueue:queue.Queue, camRunEvent:threading.Event) -> None:
        self.serialHandle = PortOpen()
        self.serialQueue = serialQueue
        self.camRunEvent = camRunEvent
        # self.posQueue = posQueue
        super().__init__()
    def run(self) -> None:
        while True:
            packed = self.serialQueue.get(block=True)
            unpacked = PackageRx(self.serialHandle, pack=packed)
            LGSRobot_MessageRespond(unpacked, self.camRunEvent)
        return super().run()

##################### LGS图像检测部分 #####################

# 面向LGS机器人的相机工作线程
class LGS_Camera(threading.Thread):
    def __init__(cam, mtx, dist, camCom:int, singleOpenTime:int, camMessageQueue:queue.Queue, camRunEvent:threading.Event) -> None:
        # 相机内参及畸变参数
        cam.mtx = mtx
        cam.dist = dist
        # 获得相机端口并开启
        cam.com = camCom
        cam.capture = cv2.VideoCapture(cam.com)
        # 设置单次运行时间
        cam.sinExeTime = singleOpenTime
        # 规划器消息队列
        cam.mesQueue = camMessageQueue
        # 外部事件指令
        cam.runEvent = camRunEvent
        # 继承父类__init__
        super().__init__()
    def run(cam) -> None:
        # 保持线程打开
        while True:
            # 等待可运行标志
            cam.runEvent.wait()
            # 开启单次运行计时器，计时结束后停止运行
            cam.timer = threading.Timer(cam.sinExeTime, cam.stop)
            cam.timer.start()
            # Cam主体任务
            while True:
                if cam.runEvent.is_set():
                    # 取得获取到的ids和tvecs
                    cam.idsGet, cam.tvecsGet = ArucoDetectionWork(cam.capture)
                else:
                    # 传输单次运行后得到的稳定的ids/tvec
                    cam.mesQueue.put(tuple([cam.idsGet, cam.tvecsGet]), block= True)
                    # 关闭窗口
                    cv2.destroyAllWindows()
                    break
        return super().run()
    def stop(cam):
        # 自行停止运行
        cam.runEvent.clear()

##################### LGS物流规划部分 #####################

# 按先列后行分配Locker
def AllocateLocker(lockers: DeliveryLocker):
    if(lockers.RowIndex == -1 & lockers.ColIndex == -1):
        return None
    # 保存此次分配的快递柜的中心坐标（世界坐标系）
    row = lockers.RowIndex
    col = lockers.ColIndex
    lockerPos = lockers.lockersSelfCoordinate[lockers.RowIndex][lockers.ColIndex]
    lockers.ColIndex += 1
    if lockers.ColIndex >= lockers.lockerCol:
        lockers.ColIndex = 0
        lockers.RowIndex += 1
    if lockers.RowIndex >= lockers.lockerRow:
        lockers.ColIndex = lockers.RowIndex = -1
    # 返回行列值
    return lockerPos, row, col

def GoodsSorting(goodsId:np.ndarray, goodsPos:np.ndarray, sortDict:dict, lockers:DeliveryLocker):
    goodsPos = np.array(goodsPos*1000, np.int16)
    # 查看id对应货物是否已经分配上快递柜
    posValue = sortDict.get('ID%d ' % goodsId)
    if posValue is None:
        # 不在的话
        # 1.寻找可以放入的快递柜
        ret = AllocateLocker(lockers)
        # 寻找成功
        # if lockerPos is not None:
        if ret is not None:
            lockerPos, row, col = ret
            # 2.添加分拣信息 {'IDi': [[gx, gy, gz], [lx, ly, lz]]} 
            # posKey = np.array((goodsPos, lockerPos*10), np.int16)
            posValue = [list(goodsPos), list(lockerPos*10), row, col]
            sortDict['ID%d ' % goodsId] = posValue
            # 3.打印分拣信息
                # print...
            # 4.回传下位机完成单次货物入柜流程需要用到的货物位置信息以及快递柜位置信息
            return posValue
        # 寻找失败（快递柜已经分配满）
        else:
            print("Lockers Full")
            # 抛回失败信息
    # 即时已经在里面了，也要对货物坐标进行更新（可能会有意外导致货物发生相对移动）
    else:
        # key_i[0]即ids[i]的货物旧坐标，此处进行更新
        posValue[0] = goodsPos
        # 重新赋值
        sortDict['ID%d ' % goodsId] = posValue
        # 回传位置信息
        return posValue

# 面向LGS机器人的快递分配线程
class LGS_Planner(threading.Thread):
    def __init__(planner, deliveryPlace:DeliveryPlace, camMessageQueue:queue.Queue) -> None:
        # 获取实际物流点信息
        planner.place = deliveryPlace
        # 获取规划信息队列
        planner.camMesQueue = camMessageQueue
        # 分类字典
        planner.sortDict = dict()
        super().__init__()
    def getSortDict(planner):
            keys = list(dict.keys(planner.sortDict))
            values = list(dict.values(planner.sortDict))
            for i in range(len(keys)):
                print("%s goods %s is assigned to locker[%d][%d]" % (keys[i], tuple(values[i][0]), values[i][-2], values[i][-1]))
    def run(planner) -> None:
        while True:
            # 得到相机消息队列中相机单次运行后稳定的ids和tvecs
            planner.goodsIds, planner.goodsTvecs = planner.camMesQueue.get(block= True)
            # 判断数据是否有效（这里假设数据为零代表单程任务结束）
            if any(planner.goodsIds) == 0:
                print("Sorting task over")
                break
            
            else:
                # 一些类型转换
                planner.goodsTvecs = np.squeeze(planner.goodsTvecs, axis= 1)
                ids_list = [int(x) for item in planner.goodsIds for x in item]
                # 找到最小id
                id_min_index = ids_list.index(min(ids_list))
                # 此处应该有一个货物分配快递柜的基于特定数学优化的算法
                # 比如基于wheel总travel最短的规划
                # 这里暂时以小id优先按先列后行分配lockers为分配方法
                # 即取ids[0]以及tvecs[0]
                print("Sorting")
                planner.curPosKey = GoodsSorting(goodsId= ids_list[id_min_index], goodsPos= planner.goodsTvecs[id_min_index],
                            sortDict= planner.sortDict, lockers =planner.place.Lockers)
                global CurPosValue
                CurPosValue = planner.curPosKey      
        return super().run()

if __name__ == '__main__':
    pass
    # CamRunEvent = threading.Event()
    # CamRunEvent.set()
    # MSI_CAM = LGS_Camera(mtx= mtx, dist= dist, CamCom= 0, SingleOpenTime= 5, runEvent= CamRunEvent)
    # MSI_CAM.start()
    # time.sleep(10)
    # CamRunEvent.set()
    # Thread_Port = Serial_Port()
    # Thread_Port.start()
    
    lockersize = np.array([30, 40, 50], np.int32)
    Lockers = DeliveryLocker(lockerSize= lockersize, lockerArray= (4, 5))
    for i in range(24):
        print(AllocateLocker(Lockers))
        time.sleep(0.2)
