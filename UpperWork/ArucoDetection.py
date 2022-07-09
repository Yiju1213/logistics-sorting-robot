import numpy
import numpy as np
import time
import cv2
import math
import cv2.aruco as aruco
from CamaraCalibration import *


def rotationVectorToEulerAngles(rvec):
    R = np.zeros((3, 3), dtype=np.float64)
    cv2.Rodrigues(rvec, R)
    sy = math.sqrt(R[0, 0] * R[0, 0] + R[1, 0] * R[1, 0])
    singular = sy < 1e-6
    if not singular:  # 偏航，俯仰，滚动
        x = math.atan2(R[2, 1], R[2, 2])
        y = math.atan2(-R[2, 0], sy)
        z = math.atan2(R[1, 0], R[0, 0])
    else:
        x = math.atan2(-R[1, 2], R[1, 1])
        y = math.atan2(-R[2, 0], sy)
        z = 0
    # 偏航，俯仰，滚动换成角度
    rx = x * 180.0 / 3.141592653589793
    ry = y * 180.0 / 3.141592653589793
    rz = z * 180.0 / 3.141592653589793
    return rx, ry, rz


# 取得先前标记校正得到的相机内参以及畸变参数
npzfile = np.load('calibrate.npz')
mtx = npzfile['mtx']
dist = npzfile['dist']


# print(mtx)
# print(dist)


# # 捕获摄像头输入
# cap = cv2.VideoCapture(0)
# print(type(cap))
# # font for displaying text (below)


def ArucoDetectionWork(capture: cv2.VideoCapture):
    font = cv2.FONT_HERSHEY_SIMPLEX
    cap = capture
    ## 捕获帧
    ret, frame = cap.read()
    
    if ret:
        ## 纠正畸变
        frame = undistortion(frame, mtx, dist)
        # print(newcameramtx)

        ## 准备检测
        # 转灰阶
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        # 取得aruco相关参数
        aruco_dict = aruco.Dictionary_get(aruco.DICT_4X4_50)
        parameters = aruco.DetectorParameters_create()
        # 还要校正？
        # dst1 = cv2.undistort(frame, mtx, dist, None, newcameramtx)

        '''
        detectMarkers(...)
            detectMarkers(image, dictionary[, corners[, ids[, parameters[, rejectedI
            mgPoints]]]]) -> corners, ids, rejectedImgPoints
        '''
        # 使用aruco.detectMarkers()函数可以检测到marker，返回ID和标志板的4个角点坐标
        corners, ids, rejectedImgPoints = aruco.detectMarkers(gray, aruco_dict, parameters=parameters)
        # if corners:
        #     print(list(corners[0][0][i] for i in range(4)))
        #     cv2.waitKey(0)
        ## 检测后进行信息处理
        if ids is not None:
            ## 面向多markers ##
            '''
            detectMarkers(corners, marker_length, CamMat, CamDistCoefs)
            - @param marker_length: Actual Length(meter) of the printed Aruco Codes.
            '''
            markerlength = 0.05
            # rvec, tvec 为所有id的ndarray
            rvec, tvec, _ = aruco.estimatePoseSingleMarkers(corners, markerlength, mtx, dist)
            # print(type(rvec))
            # print(rvec)
            # print(tvec)
            # cv2.waitKey(0)
            # 估计每个标记的姿态并返回nt(值rvet和tvec ---不同
            # from camera coeficcients
            (rvec-tvec).any()  # get rid of that nasty numpy value array error
            for i in range(rvec.shape[0]):
                aruco.drawAxis(frame, mtx, dist, rvec[i, :, :], tvec[i, :, :], 0.03)
                # 框起来
                aruco.drawDetectedMarkers(frame, corners)
            for specificID in range(len(ids)):
                ###### DRAW ID #####
                idCordinate = np.uint32(corners[specificID][0][0]) + (0, -50)
                transCordinate = np.uint32(corners[specificID][0][0]) + (0, -30)
                revCordinate = np.uint32(corners[specificID][0][0]) + (0, -10)
                cv2.putText(frame, "Id: " + str(ids[specificID]), idCordinate, font, 0.6, (150, 100, 220), 2, cv2.LINE_AA)
                singleRvec = numpy.expand_dims(rvec[specificID], 0)
                singleTvec = numpy.expand_dims(tvec[specificID], 0)
                eulerAngles = rotationVectorToEulerAngles(singleRvec)
                eulerAngles = [round(i, 2) for i in eulerAngles]
                # cv2.putText(frame, str(eulerAngles), revCordinate, font, 0.4, (0, 255, 0), 2, cv2.LINE_AA)
                singleTvec = singleTvec * 1000 # 转换成毫米
                for i in range(3):
                    singleTvec[0][0][i] = round(singleTvec[0][0][i], 1)
                singleTvec = np.squeeze(singleTvec)
                cv2.putText(frame, str(singleTvec), transCordinate, font, 0.4, (0, 255, 0), 2,
                            cv2.LINE_AA)
            
        else:
            ##### DRAW "NO IDS" #####
            cv2.putText(frame, "No Ids", (0, 40), font, 0.6, (0, 255, 0), 2, cv2.LINE_AA)
            

    # 显示结果框架
    cv2.imshow("gray", gray)
    cv2.imshow("frame", frame)

    key = cv2.waitKey(1)

    # if key == 27:         # 按esc键退出
    #     print('esc break...')
    #     cap.release()
    #     cv2.destroyAllWindows()
    #     break

    num = 0
    if key == ord(' '):  # 按空格键保存
        filename = "E:/code/" + str(time.time())[:10] + ".jpg"
        num += 1
        cv2.imwrite(filename, frame)
        print("ok")
        
    if ids is None:
        return [0], [0]
    return ids, tvec


if __name__ == '__main__':
    cap = cv2.VideoCapture(0)
    while True:
        ArucoDetectionWork(cap)
