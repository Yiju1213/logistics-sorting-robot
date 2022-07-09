import os
from time import sleep
import numpy as np
import cv2

def calibrate():
    # ?
    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)
    # 标定板棋盘格检测内点
    Nx_cor = 10 - 1
    Ny_cor =  7 - 1
    # ?
    objp = np.zeros((Nx_cor * Ny_cor, 3), np.float32)
    # ?
    objp[:, :2] = np.mgrid[0:Nx_cor, 0:Ny_cor].T.reshape(-1, 2)
    objpoints = []  # 3d points in real world space.
    imgpoints = []  # 2d points in image plane.

    count = 0  # count 用来标志成功检测到的棋盘格画面数量
    while (True):

        ret, frame = cap.read()
        # 检测到空格输入才会进入
        if cv2.waitKey(1) & 0xFF == ord(' '):
            print(count)
            # Our operations on the frame come here
            # 转灰阶
            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            # 查找棋盘格角点信息
            ret, corners = cv2.findChessboardCorners(gray, (Nx_cor, Ny_cor), None)  # Find the corners
            # If found, add object points, image points
            if ret == True:
                # cornerSubPix -@brief Refines the corner locations.
                corners = cv2.cornerSubPix(gray, corners, (5, 5), (-1, -1), criteria)
                objpoints.append(objp)
                imgpoints.append(corners)
                # 在原始帧中画出角点信息
                cv2.drawChessboardCorners(frame, (Nx_cor, Ny_cor), corners, ret)
                count += 1
                if count > 20:
                    break

        # Display the resulting frame
        cv2.imshow('frame', frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
        
    # 声明此处使用的变量是外部全局变量
    global mtx, dist
    # 标定，mtx 是相机内参，dist 是畸变，rvecs,tvecs 分别是旋转矩阵和平移矩阵代表外参
    ret, mtx, dist, rvecs, tvecs = cv2.calibrateCamera(objpoints, imgpoints, gray.shape[::-1], None, None)
    print(mtx, dist)
    
    mean_error = 0
    for i in range(len(objpoints)):
        imgpoints2, _ = cv2.projectPoints(objpoints[i], rvecs[i], tvecs[i], mtx, dist)
        error = cv2.norm(imgpoints[i], imgpoints2, cv2.NORM_L2) / len(imgpoints2)
        mean_error += error

    print ("total error: ", mean_error / len(objpoints))
        # # When everything done, release the capture
    # 畸变参数{k1, k2, p1, p2, k3}，但一般只用到前四个 ([0:i]左闭右开)
    np.savez('calibrate.npz', mtx=mtx, dist=dist[0:4])

def undistortion(img, mtx, dist):
    # 读取图片高[0]、宽[1]
    h, w = img.shape[:2]
    # getOptimalNewCameraMatrix -@brief Returns the new camera matrix based on the free scaling parameter.
    newcameramtx, roi = cv2.getOptimalNewCameraMatrix(mtx, dist, (w, h), 1, (w, h))
    # undistort -@brief Transforms an image to compensate for lens distortion.
    dst = cv2.undistort(img, mtx, dist, None, newcameramtx)

    # crop the image
    x, y, w, h = roi
    if roi != (0, 0, 0, 0):
        dst = dst[y:y + h, x:x + w]

    return dst

# 单独调试时会使用到
if __name__ == '__main__':
    # 打开电脑摄像头
    cap = cv2.VideoCapture(0)
    
    mtx = []
    dist = []
    
    # 之前标定过就无需再次标定
    try:
        npzfile = np.load('calibrate.npz')
        mtx = npzfile['mtx']
        dist = npzfile['dist']
    # 未标定过就标定一下
    except IOError:
        print("now calibrate")
        calibrate()
        print("calibrate over")
    
    
    print('dist', dist[0:4])
    
    while (True):

        ret, frame = cap.read()

        frame = undistortion(frame, mtx, dist[0:4])
        # Display the resulting frame
        cv2.imshow('frame', frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()