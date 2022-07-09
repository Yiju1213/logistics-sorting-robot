#ifndef _APP_
#define _APP_

#include "stdint.h"

/* Defines */
#define True 1
#define False 0

#define pi 3.141592653
#define SIGN(x) ((x) > 0 ? (1) : (-1))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define COOR_BYTE 6

#define OFFSET_RtoZ 100
#define OFFSET_RtoT 50		// R轴到Theta轴偏心距 

// 单位mm
#define GOODS_OFFSET_X	50
#define GOODS_OFFSET_Y	(-40)
#define GOODS_OFFSET_Z	30
#define WORLD_OFFSET_X	(-50)
#define WORLD_OFFSET_Y	(-60)
#define WORLD_OFFSET_Z	(-300)

#define THETA_PARALLEL_Y	(pi/2)	// rad/s
#define RAXIS_LENGTH		290		// mm

#define MAX_WHEEL_VEL 225	// mm/s
#define MAX_RAXIS_VEL 377	// mm/s
#define MAX_THETA_VEL (pi*57/180) // rad/s
#define MAX_ZAXIS_VEL 12		// mm/s

#define CAPULA_LIMIT_FORCE		100 	// g
#define GOODS_LIMIT_FORCE		(-200)	// g	

#define WALL_LIMIT_LEN	50	// mm
#define RELEASE_TIME 5
void WaitSYNCfromUpper(void);
void WaitCameraOPEN(void);
void GetCoordinateMessage(void);
void RobotIkine(void);
uint8_t IfLockerNotSame(void);
void SetPressureSensorWork(int32_t limitForce);
void WaitPressureSensorWorkEnd(void);
uint8_t IfGoodsDrops(void);
void SetRangingSensorWork(float limitLen);
void WaitRangingSensorWorkEnd(void);
uint8_t SendJointMoveOrder(uint8_t jointName, int16_t tarPos, int16_t tarVel);

#endif
