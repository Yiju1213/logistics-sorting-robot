import math
import numpy as np
from LogisticStuffClass import DeliveryLocker

class Joint():
    # 
    def __init__(self, name:np.uint8):
        self.name = name
    # 用uint8保存bit值用于字节流传输
    def setTarPos(self, tarPos:np.int16):
        self.TarPos = tarPos
        self.TarPosByteH = np.uint8(tarPos >> 8)
        self.TarPosByteL = np.uint8(tarPos & 0xFF)
    def setTarVel(self, tarVel:np.int16):
        self.TarVel = tarVel
        self.TarVelByteH = np.uint8(tarVel >> 8)
        self.TarVelByteL = np.uint8(tarVel & 0xFF)
    def getJointArray(self):
        return np.array([self.name, self.TarPosByteH, self.TarPosByteL, 
                            self.TarVelByteH, self.TarVelByteL], np.uint8)

class LogisticSortRobot():
    ### Joint ID    |   ID1   |   ID2   |   ID3   |   ID4   ###
    ### Joint Info  |  JWHEEL |  JZAXIS |  JRAXIS |  JTHETA ###
    def __init__(robot, offset_RaxisToZaxis:float, offsetVec_CamToZaxis:np.ndarray) -> None:
        # 关节 用于记录各关节逆解值
        robot.jWheel = Joint(1)
        robot.jZaxis = Joint(2)
        robot.jRaxis = Joint(3)
        robot.jTheta = Joint(4)
        # R轴到Z轴偏距
        robot.offset_RaxisToZaxis = offset_RaxisToZaxis
        # 相机坐标系（垂直于Z轴）到Z轴偏距向量(offsetVec {ox, oy, oz})
        robot.offsetVec_CamToZaxis = offsetVec_CamToZaxis
    # 基于快递柜位置的轮关节逆解
    def ikine_jWheel(robot, locker_x:np.int16):
        # 逆解
        tarPos = locker_x - robot.offset_RaxisToZaxis
        # 设置位移
        robot.jWheel.setTarPos(tarPos)
        # 设置速度?
        # robot.jWheel.setTarVel()
    def ikine_jZaxis(robot, locker_z:np.int16):
        tarPos = locker_z
        robot.jZaxis.setTarPos(tarPos)
        # Z轴有两个关键点
    def ikine_RTplane(robot, goodsPos:np.ndarray):
        # 传进来的goodsPos是相对于CAM坐标系的
        # CAM坐标系关于Z轴顶点为零点的XY平面有平移变换offsetVec_CamToZaxis
        # 先将goodsPos转换到机器坐标系
        goodsPos = goodsPos + robot.offsetVec_CamToZaxis 
        [gx, gy, gz, e] = [goodsPos[0], goodsPos[1], goodsPos[2], robot.offset_RaxisToZaxis]
        raxis_tarPos = l = math.sqrt(gx^2 + gy^2 - e^2)
        theta_tarPos = math.atan2(l, e) + math.atan2(gy, gx)
        robot.jRaxis.setTarPos()
        
    
    
    





        
        

