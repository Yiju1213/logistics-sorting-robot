import numpy as np

class DeliveryLocker():
    def __init__(self, lockerSize:np.ndarray, lockerArray:np.ndarray) -> None:
        # lockerSize 指代单格快递柜三维尺寸
        # Width  refers to x Axis
        # Height refers to z Axis
        # Depth  refers to y Axis
        self.lockerWidth    = lockerSize[0]
        self.lockerHeight   = lockerSize[1]
        self.lockerDepth    = lockerSize[2]
        # lockerArray 指代快递柜阵列
        self.lockerRow = lockerArray[0]
        self.lockerCol = lockerArray[1]
        # 用于分配Lockers时作为占用指标
        self.RowIndex = 0
        self.ColIndex = 0
        # 自动调用getLockersMidCoordinate获取自身坐标系下快递柜坐标
        self.getLockersMidCoordinate()
    def getLockersMidCoordinate(self):
        # row, col, 3 维数组用来存放快递柜阵列的坐标
        self.lockersSelfCoordinate = np.zeros((self.lockerRow, self.lockerCol, 3), np.int32)
        for row in range(self.lockerRow):
            for col in range(self.lockerCol):
                x = (0.5 + row) * self.lockerWidth
                y = 0
                z = (0.5 + col) * self.lockerHeight
                specific = np.array([x, y, z], np.int32)
                self.lockersSelfCoordinate[row][col] = specific

class DeliveryPlace():
    def __init__(self, lockers:DeliveryLocker) -> None:
        # 获得实例对象对应快递柜
        self.Lockers = lockers
    def setZerosOffset(self, offset:np.ndarray):
        # 获取快递柜坐标系相对于快递车零位点的偏置
        # 零位点主要针对x/y Axis进行偏置
        self.offset = np.full((self.Lockers.lockerRow, self.Lockers.lockerCol, 3), offset, dtype=np.int32)
        # print("offset:\n", self.offset)
        # 将Locker自身坐标系转到世界坐标系
        self.lockersWorldCoordinate = self.Lockers.lockersSelfCoordinate + self.offset

        
        


if __name__ == '__main__':
    lockersize = np.array([30, 40, 50], np.int32)
    Lockers = DeliveryLocker(lockerSize= lockersize, lockerArray= (2, 2))
    print("Lockers Self Coordinate:\n", Lockers.lockersSelfCoordinate)
    zerosOffset = np.array([30, 30, 0], np.int32)
    Place = DeliveryPlace(lockers= Lockers)
    Place.getZerosOffset(offset= zerosOffset)
    print("Lockers Worlds Coordinate:\n", Place.lockersWorldCoordinate)
    
    
        
    
            
