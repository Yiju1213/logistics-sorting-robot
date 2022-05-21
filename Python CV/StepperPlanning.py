from numpy import int16, int8, uint8
import numpy


class Stepper:
    def __init__(self, name:uint8):
        self.name = name
        self.TarPos = 0
        self.TarPosByteL = 0
        self.TarPosByteH = 0
    def SetTarPos(self, tarPos:int16):
        self.TarPos = tarPos
        # 用uint8保存bit值方便位运算
        self.TarPosByteH = uint8(tarPos >> 8)
        self.TarPosByteL = uint8(tarPos & 0xFF)
        self.TarPosArray = numpy.array([self.name, self.TarPosByteH, self.TarPosByteL], uint8)

