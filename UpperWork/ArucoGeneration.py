import cv2
import cv2.aruco as aruco
import numpy as np

aruco_dict = aruco.Dictionary_get(aruco.DICT_4X4_50)
arucoId: int = 6
tag = np.zeros((600, 600, 1), dtype=np.uint8)
cv2.aruco.drawMarker(aruco_dict, arucoId, 600, tag, 1)
cv2.imwrite('ID%d.jpg' % arucoId , tag)
cv2.imshow(winname="ArUCo Tag (ID:%d)" % arucoId, mat=tag)
cv2.waitKey(0)

