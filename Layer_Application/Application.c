
/* Includes */
#include "Application.h"
#include "TaskExecute.h"
#include "math.h"
#include "string.h"

/* Semaphores */
extern osSemaphoreId SYNC_SignalHandle;
extern osSemaphoreId CameraOpen_SigalHandle;

/* Queues */
extern QueueHandle_t Coordinate_Queue;
extern QueueHandle_t JointDrive_Queue;
extern QueueHandle_t PressureSensor_Queue;
extern QueueHandle_t RangingSensor_Queue;
/* Globals */
uint8_t CoordinateArray[COOR_BYTE], WaitTime[2];
int16_t Locker_x, Locker_y, Locker_z, Goods_x, Goods_y, Goods_z;
float	WheelPos, ZaxisPosToGoods, ZaxisPosToLocker, RaxisPos, ThetaPos;

/* Tasks */
/** @brife 应用程序任务-面向过程任务流程 */
void AppTaskFunc(void const * argument)
{
	/* 0.一批物流分拣前的准备 */
	// 等待外设初始化
	osDelay(10);
	// 先取走信号量，等待上位机发送同步指令后SerialTaskFunc释放
	xSemaphoreTake(SYNC_SignalHandle, portMAX_DELAY);
	// 同理，等待ESP32-CAM打开后由上位机处理端下发指令释放
	xSemaphoreTake(CameraOpen_SigalHandle, portMAX_DELAY);
	
	/** 1.向远端上位机发送同步指令并等待同步 */
	SerialRequireFromUpper(GET_SYNC);
	WaitSYNCfromUpper();
	
	/** 2.开启摄像头并等待摄像头开启信号 */ 
	/** @Tips 无ESP32-CAM，用PC摄像头代替，即向上位机发送开启指令 */
	SerialRequireFromUpper(OPEN_CAM); 
	WaitCameraOPEN();
	
	// 进入每个快递入柜的循环操作
	for(;;)
	{
		do
		{
			/** 3.机构复位！ */
			SendJointMoveOrder(ALL_JOINT_RESET, 0, 0);
			
			/** 4.向远端上位机获取基于快递码识别以及快递分配后得到的此次交互的快递坐标以及快递柜坐标 */ 
			SerialRequireFromUpper(GET_MESSAGE);
			GetCoordinateMessage();
			
			do
			{
				/** 5.得到的坐标信息进行位置逆解，结果放入全局变量中 */
				RobotIkine(); 
				
				/** 6.下发信息控制轮关节移动到快递柜中心面，并等待完成 */
				WaitTime[0] = SendJointMoveOrder(WHEEL_JOINT, WheelPos, SIGN(WheelPos)*MAX_WHEEL_VEL);
				osDelay(WaitTime[0]);
				
				/** 7.轮关节运动完成后，再次向远端上位机获取本次交互快递坐标以及快递柜坐标，
					目的是为了修正轮关节移动可能造成的快递位移（快递柜坐标应是不变的）
					以及检测是否出现当前快递翻倒码面不朝上导致检测不到的情况 */
				SerialRequireFromUpper(GET_MESSAGE);
				GetCoordinateMessage();
				/** @Tips 同一快递id会被分配到唯一快递柜，如果本次回传的快递柜坐标与上次不一样，
					说明要放置的快递变了，则（大概率）说明原快递倒了，码面未朝上检测不到了，
					就只能重新回到轮关节控制移动到新的快递柜中心面，原快递将在最后交由人工处理 */
			}while(IfLockerNotSame());
			
			/** 8.下发信息控制RAXIS/THETA关节移动到快递上方，并等待完成 */
			WaitTime[0] = SendJointMoveOrder(RAXIS_JOINT, RaxisPos, SIGN(RaxisPos)*MAX_RAXIS_VEL);
			WaitTime[1] = SendJointMoveOrder(THETA_JOINT, ThetaPos, SIGN(ThetaPos)*MAX_THETA_VEL);
			osDelay(MAX(WaitTime[0],WaitTime[1]));
			
			/** 9.下发信息控制ZAXIS关节快速向下移动至CAM检测的快递高度上方5mm，并等待完成，
				同时预先开启气泵 */
			WaitTime[0] = SendJointMoveOrder(ZAXIS_JOINT, (ZaxisPosToGoods + 5),SIGN(ZaxisPosToGoods)*MAX_ZAXIS_VEL);
			PUMP_ON;
			osDelay(WaitTime[0]);
			
			/** 10.下发信息控制ZAXIS关节慢速向下移动，同时给压力传感工作信号开始工作，等待
				吸盘下压力达到要求，随机停止运动 */
			SendJointMoveOrder(ZAXIS_JOINT, NoPosTar, (-0.1 * MAX_ZAXIS_VEL));
			SetPressureSensorWork(CAPULA_LIMIT_FORCE);
			WaitPressureSensorWorkEnd();
			SendJointMoveOrder(ZAXIS_JOINT, NoPosTar, 0);
			osDelay(10);	// 确保ZAXIS即时停止
			/** 11.吸盘下压力达到要求后，下发ZAXIS关节控制任务-快速向上移动至快递柜高度，同时检测
				压力传感器是否为负值（负值说明快递未掉落）*/
			WaitTime[0] = SendJointMoveOrder(ZAXIS_JOINT, ZaxisPosToLocker, SIGN(ZaxisPosToLocker)*MAX_ZAXIS_VEL);
			osDelay(10);	// 确保ZAXIS已经在向上运动，保证快递未掉落情况一定为负值
			SetPressureSensorWork(GOODS_LIMIT_FORCE);
			osDelay(WaitTime[0]);
			/** @Tips ZAXIS上移过程压力传感器一直在工作，且有限额（如-100g），当大于此值时，说明快递掉落，
				IfGoodsDrops()就是用于检测是否从压力传感器任务中接收到掉落消息的函数，如果接收到掉落消息，
				则应重新回到复位点，并重新从摄像头获取规划分配的坐标信息 */
		}while(IfGoodsDrops());
		
		/** 12. ZAXIS关节上升到LockerHeight后，将THETA轴移动到与Y轴平行（相对零位位置是pi/2） */
		WaitTime[0] = SendJointMoveOrder(THETA_JOINT, THETA_PARALLEL_Y, SIGN(THETA_PARALLEL_Y)*MAX_THETA_VEL);
		osDelay(WaitTime[0]);
		
		/** 13.THETA轴平行Y轴后，驱动R轴向快递柜里运行，同时打开测距传感器检测到快递柜内墙距离20mm时停止 */
		/** @Caution 未考虑行程不足的情况 */
		SendJointMoveOrder(RAXIS_JOINT, NoPosTar, (1 * MAX_RAXIS_VEL));
		SetRangingSensorWork(WALL_LIMIT_LEN);
		WaitRangingSensorWorkEnd();
		SendJointMoveOrder(RAXIS_JOINT, NoPosTar, 0);
		osDelay(10);		// 确保及时停止
		
		/** 14.关气泵，同时解除吸盘负压，等待一定时间后Z轴向上移动一小段距离，随后R轴送出直到最远端方便复位 */
		PUMP_OFF;
		PUMP_Release_On;
		osDelay(RELEASE_TIME);
		PUMP_Release_Off;
		WaitTime[0] = SendJointMoveOrder(ZAXIS_JOINT, (ZaxisPosToLocker + 10), SIGN(ZaxisPosToLocker)*MAX_ZAXIS_VEL);
		osDelay(WaitTime[0]);
		WaitTime[0] = SendJointMoveOrder(RAXIS_JOINT, 0, -1 * MAX_RAXIS_VEL);
		osDelay(WaitTime[0]);
		/** 一次快递入柜完成 */
	}
}

/** @brife 等待上位机同步信号，换个函数名方便任务流程叙述 */
void WaitSYNCfromUpper(void)
{
	xSemaphoreTake(SYNC_SignalHandle, portMAX_DELAY);
}

/** @brife 等待摄像头开启信号，换个函数名方便任务流程叙述 */
void WaitCameraOPEN(void)
{
	xSemaphoreTake(CameraOpen_SigalHandle, portMAX_DELAY);
}
/** @brife 接收坐标信息并放入全局数组CoordinateArray，换个函数名方便任务流程叙述 */
void GetCoordinateMessage(void)
{
	xQueueReceive(Coordinate_Queue,(void*)&CoordinateArray, portMAX_DELAY);
}
/** @brife 开启压力传感器工作并设置临界点 */
void SetPressureSensorWork(int32_t limitForce)
{
	xQueueSend(PressureSensor_Queue, &limitForce, portMAX_DELAY);
}
/** @brife 等待压力传感器达到工作目标返回信号 */
void WaitPressureSensorWorkEnd(void)
{
	int32_t message;
	xQueueReceive(PressureSensor_Queue, (void*)&message, portMAX_DELAY);
}
/** @brife 开启测距传感器测距工作 */
void SetRangingSensorWork(float limitLen)
{
	xQueueSend(RangingSensor_Queue, &limitLen, portMAX_DELAY);
}
/** @brife 等待测距传感器达到工作目标返回信号 */
void WaitRangingSensorWorkEnd(void)
{
	float message;
	xQueueReceive(RangingSensor_Queue, (void*)&message, portMAX_DELAY);
}
/** @brife 坐标信息逆解至机器运行过程中的关键点位运动量 */
void RobotIkine(void)
{
	// 坐标信息录入
	/** @Tips 货物坐标在CAM坐标系下得到，而CAM坐标系关于机器坐标系有平移变换
		offsetVec_CamToThetaAxis，即货物坐标需要进行坐标变换 */
	Goods_x = CoordinateArray[0] + GOODS_OFFSET_X;
	Goods_y = CoordinateArray[1] + GOODS_OFFSET_Y;
	Goods_z = CoordinateArray[2] + GOODS_OFFSET_Z;
	/** @Tips 快递柜所处世界坐标系关于机器坐标系同样有平移变换offsetVec_WorldToThetaAxis
		即快递柜坐标同样需要进行坐标变换 */
	Locker_x = CoordinateArray[3] + WORLD_OFFSET_X;
	Locker_y = CoordinateArray[4] + WORLD_OFFSET_Y;
	Locker_z = CoordinateArray[5] + WORLD_OFFSET_Z;
	// 轮关节位置逆解
	WheelPos = Locker_x - OFFSET_RtoZ;
	// Z轴两个关键点位逆解
	ZaxisPosToLocker = Locker_z;
	ZaxisPosToGoods = Goods_z;
	// R-Theta极坐标平面逆解
	float l, e = OFFSET_RtoT;
	l = sqrt(pow(Goods_x, 2) + pow(Goods_y, 2) - pow(e, 2));
	RaxisPos = RAXIS_LENGTH - l;
	ThetaPos = -(atan2(l, e) - pi/2); // 从逆时针为正转为顺时针为正
}
/** @brife 通过队列下发关节控制指令 */
uint8_t SendJointMoveOrder(uint8_t jointName, int16_t tarPos, int16_t tarVel)
{
	uint8_t pack[5];
	memcpy(&pack[0], &jointName, 1);
	memcpy(&pack[1], &tarPos, 2);
	memcpy(&pack[3], &tarVel, 2);
	xQueueSend(JointDrive_Queue, pack, portMAX_DELAY);
	uint8_t time = fabs(ceil(tarPos/tarVel));
	return time;
}
/** @brife 检测是否是同一快递 */
uint8_t IfLockerNotSame(void)
{
	if(Locker_x == CoordinateArray[3] + WORLD_OFFSET_X)
		// CoordinateArray[4] 都是0
		// if(Locker_y == CoordinateArray[4] + LOCKER_OFFSET_Y) 
		if(Locker_z == CoordinateArray[5] + WORLD_OFFSET_Z)
			return False;
	return True;
}
/** @brife 检测快递是否掉落 */
uint8_t IfGoodsDrops(void)
{
	int32_t limitForce;
	// 没有接收到说明快递未掉落，此时返回False
	// 如果接收到了，说明快递掉落，返回True
	return xQueueReceive(PressureSensor_Queue, (void*)&limitForce, 0);
}
