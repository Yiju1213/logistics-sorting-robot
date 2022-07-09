
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
/** @brife Ӧ�ó�������-��������������� */
void AppTaskFunc(void const * argument)
{
	/* 0.һ�������ּ�ǰ��׼�� */
	// �ȴ������ʼ��
	osDelay(10);
	// ��ȡ���ź������ȴ���λ������ͬ��ָ���SerialTaskFunc�ͷ�
	xSemaphoreTake(SYNC_SignalHandle, portMAX_DELAY);
	// ͬ���ȴ�ESP32-CAM�򿪺�����λ��������·�ָ���ͷ�
	xSemaphoreTake(CameraOpen_SigalHandle, portMAX_DELAY);
	
	/** 1.��Զ����λ������ͬ��ָ��ȴ�ͬ�� */
	SerialRequireFromUpper(GET_SYNC);
	WaitSYNCfromUpper();
	
	/** 2.��������ͷ���ȴ�����ͷ�����ź� */ 
	/** @Tips ��ESP32-CAM����PC����ͷ���棬������λ�����Ϳ���ָ�� */
	SerialRequireFromUpper(OPEN_CAM); 
	WaitCameraOPEN();
	
	// ����ÿ���������ѭ������
	for(;;)
	{
		do
		{
			/** 3.������λ�� */
			SendJointMoveOrder(ALL_JOINT_RESET, 0, 0);
			
			/** 4.��Զ����λ����ȡ���ڿ����ʶ���Լ���ݷ����õ��Ĵ˴ν����Ŀ�������Լ���ݹ����� */ 
			SerialRequireFromUpper(GET_MESSAGE);
			GetCoordinateMessage();
			
			do
			{
				/** 5.�õ���������Ϣ����λ����⣬�������ȫ�ֱ����� */
				RobotIkine(); 
				
				/** 6.�·���Ϣ�����ֹؽ��ƶ�����ݹ������棬���ȴ���� */
				WaitTime[0] = SendJointMoveOrder(WHEEL_JOINT, WheelPos, SIGN(WheelPos)*MAX_WHEEL_VEL);
				osDelay(WaitTime[0]);
				
				/** 7.�ֹؽ��˶���ɺ��ٴ���Զ����λ����ȡ���ν�����������Լ���ݹ����꣬
					Ŀ����Ϊ�������ֹؽ��ƶ�������ɵĿ��λ�ƣ���ݹ�����Ӧ�ǲ���ģ�
					�Լ�����Ƿ���ֵ�ǰ��ݷ������治���ϵ��¼�ⲻ������� */
				SerialRequireFromUpper(GET_MESSAGE);
				GetCoordinateMessage();
				/** @Tips ͬһ���id�ᱻ���䵽Ψһ��ݹ�������λش��Ŀ�ݹ��������ϴβ�һ����
					˵��Ҫ���õĿ�ݱ��ˣ��򣨴���ʣ�˵��ԭ��ݵ��ˣ�����δ���ϼ�ⲻ���ˣ�
					��ֻ�����»ص��ֹؽڿ����ƶ����µĿ�ݹ������棬ԭ��ݽ���������˹����� */
			}while(IfLockerNotSame());
			
			/** 8.�·���Ϣ����RAXIS/THETA�ؽ��ƶ�������Ϸ������ȴ���� */
			WaitTime[0] = SendJointMoveOrder(RAXIS_JOINT, RaxisPos, SIGN(RaxisPos)*MAX_RAXIS_VEL);
			WaitTime[1] = SendJointMoveOrder(THETA_JOINT, ThetaPos, SIGN(ThetaPos)*MAX_THETA_VEL);
			osDelay(MAX(WaitTime[0],WaitTime[1]));
			
			/** 9.�·���Ϣ����ZAXIS�ؽڿ��������ƶ���CAM���Ŀ�ݸ߶��Ϸ�5mm�����ȴ���ɣ�
				ͬʱԤ�ȿ������� */
			WaitTime[0] = SendJointMoveOrder(ZAXIS_JOINT, (ZaxisPosToGoods + 5),SIGN(ZaxisPosToGoods)*MAX_ZAXIS_VEL);
			PUMP_ON;
			osDelay(WaitTime[0]);
			
			/** 10.�·���Ϣ����ZAXIS�ؽ����������ƶ���ͬʱ��ѹ�����й����źſ�ʼ�������ȴ�
				������ѹ���ﵽҪ�����ֹͣ�˶� */
			SendJointMoveOrder(ZAXIS_JOINT, NoPosTar, (-0.1 * MAX_ZAXIS_VEL));
			SetPressureSensorWork(CAPULA_LIMIT_FORCE);
			WaitPressureSensorWorkEnd();
			SendJointMoveOrder(ZAXIS_JOINT, NoPosTar, 0);
			osDelay(10);	// ȷ��ZAXIS��ʱֹͣ
			/** 11.������ѹ���ﵽҪ����·�ZAXIS�ؽڿ�������-���������ƶ�����ݹ�߶ȣ�ͬʱ���
				ѹ���������Ƿ�Ϊ��ֵ����ֵ˵�����δ���䣩*/
			WaitTime[0] = SendJointMoveOrder(ZAXIS_JOINT, ZaxisPosToLocker, SIGN(ZaxisPosToLocker)*MAX_ZAXIS_VEL);
			osDelay(10);	// ȷ��ZAXIS�Ѿ��������˶�����֤���δ�������һ��Ϊ��ֵ
			SetPressureSensorWork(GOODS_LIMIT_FORCE);
			osDelay(WaitTime[0]);
			/** @Tips ZAXIS���ƹ���ѹ��������һֱ�ڹ����������޶��-100g���������ڴ�ֵʱ��˵����ݵ��䣬
				IfGoodsDrops()�������ڼ���Ƿ��ѹ�������������н��յ�������Ϣ�ĺ�����������յ�������Ϣ��
				��Ӧ���»ص���λ�㣬�����´�����ͷ��ȡ�滮�����������Ϣ */
		}while(IfGoodsDrops());
		
		/** 12. ZAXIS�ؽ�������LockerHeight�󣬽�THETA���ƶ�����Y��ƽ�У������λλ����pi/2�� */
		WaitTime[0] = SendJointMoveOrder(THETA_JOINT, THETA_PARALLEL_Y, SIGN(THETA_PARALLEL_Y)*MAX_THETA_VEL);
		osDelay(WaitTime[0]);
		
		/** 13.THETA��ƽ��Y�������R�����ݹ������У�ͬʱ�򿪲�ഫ������⵽��ݹ���ǽ����20mmʱֹͣ */
		/** @Caution δ�����г̲������� */
		SendJointMoveOrder(RAXIS_JOINT, NoPosTar, (1 * MAX_RAXIS_VEL));
		SetRangingSensorWork(WALL_LIMIT_LEN);
		WaitRangingSensorWorkEnd();
		SendJointMoveOrder(RAXIS_JOINT, NoPosTar, 0);
		osDelay(10);		// ȷ����ʱֹͣ
		
		/** 14.�����ã�ͬʱ������̸�ѹ���ȴ�һ��ʱ���Z�������ƶ�һС�ξ��룬���R���ͳ�ֱ����Զ�˷��㸴λ */
		PUMP_OFF;
		PUMP_Release_On;
		osDelay(RELEASE_TIME);
		PUMP_Release_Off;
		WaitTime[0] = SendJointMoveOrder(ZAXIS_JOINT, (ZaxisPosToLocker + 10), SIGN(ZaxisPosToLocker)*MAX_ZAXIS_VEL);
		osDelay(WaitTime[0]);
		WaitTime[0] = SendJointMoveOrder(RAXIS_JOINT, 0, -1 * MAX_RAXIS_VEL);
		osDelay(WaitTime[0]);
		/** һ�ο�������� */
	}
}

/** @brife �ȴ���λ��ͬ���źţ��������������������������� */
void WaitSYNCfromUpper(void)
{
	xSemaphoreTake(SYNC_SignalHandle, portMAX_DELAY);
}

/** @brife �ȴ�����ͷ�����źţ��������������������������� */
void WaitCameraOPEN(void)
{
	xSemaphoreTake(CameraOpen_SigalHandle, portMAX_DELAY);
}
/** @brife ����������Ϣ������ȫ������CoordinateArray���������������������������� */
void GetCoordinateMessage(void)
{
	xQueueReceive(Coordinate_Queue,(void*)&CoordinateArray, portMAX_DELAY);
}
/** @brife ����ѹ�������������������ٽ�� */
void SetPressureSensorWork(int32_t limitForce)
{
	xQueueSend(PressureSensor_Queue, &limitForce, portMAX_DELAY);
}
/** @brife �ȴ�ѹ���������ﵽ����Ŀ�귵���ź� */
void WaitPressureSensorWorkEnd(void)
{
	int32_t message;
	xQueueReceive(PressureSensor_Queue, (void*)&message, portMAX_DELAY);
}
/** @brife ������ഫ������๤�� */
void SetRangingSensorWork(float limitLen)
{
	xQueueSend(RangingSensor_Queue, &limitLen, portMAX_DELAY);
}
/** @brife �ȴ���ഫ�����ﵽ����Ŀ�귵���ź� */
void WaitRangingSensorWorkEnd(void)
{
	float message;
	xQueueReceive(RangingSensor_Queue, (void*)&message, portMAX_DELAY);
}
/** @brife ������Ϣ������������й����еĹؼ���λ�˶��� */
void RobotIkine(void)
{
	// ������Ϣ¼��
	/** @Tips ����������CAM����ϵ�µõ�����CAM����ϵ���ڻ�������ϵ��ƽ�Ʊ任
		offsetVec_CamToThetaAxis��������������Ҫ��������任 */
	Goods_x = CoordinateArray[0] + GOODS_OFFSET_X;
	Goods_y = CoordinateArray[1] + GOODS_OFFSET_Y;
	Goods_z = CoordinateArray[2] + GOODS_OFFSET_Z;
	/** @Tips ��ݹ�������������ϵ���ڻ�������ϵͬ����ƽ�Ʊ任offsetVec_WorldToThetaAxis
		����ݹ�����ͬ����Ҫ��������任 */
	Locker_x = CoordinateArray[3] + WORLD_OFFSET_X;
	Locker_y = CoordinateArray[4] + WORLD_OFFSET_Y;
	Locker_z = CoordinateArray[5] + WORLD_OFFSET_Z;
	// �ֹؽ�λ�����
	WheelPos = Locker_x - OFFSET_RtoZ;
	// Z�������ؼ���λ���
	ZaxisPosToLocker = Locker_z;
	ZaxisPosToGoods = Goods_z;
	// R-Theta������ƽ�����
	float l, e = OFFSET_RtoT;
	l = sqrt(pow(Goods_x, 2) + pow(Goods_y, 2) - pow(e, 2));
	RaxisPos = RAXIS_LENGTH - l;
	ThetaPos = -(atan2(l, e) - pi/2); // ����ʱ��Ϊ��תΪ˳ʱ��Ϊ��
}
/** @brife ͨ�������·��ؽڿ���ָ�� */
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
/** @brife ����Ƿ���ͬһ��� */
uint8_t IfLockerNotSame(void)
{
	if(Locker_x == CoordinateArray[3] + WORLD_OFFSET_X)
		// CoordinateArray[4] ����0
		// if(Locker_y == CoordinateArray[4] + LOCKER_OFFSET_Y) 
		if(Locker_z == CoordinateArray[5] + WORLD_OFFSET_Z)
			return False;
	return True;
}
/** @brife ������Ƿ���� */
uint8_t IfGoodsDrops(void)
{
	int32_t limitForce;
	// û�н��յ�˵�����δ���䣬��ʱ����False
	// ������յ��ˣ�˵����ݵ��䣬����True
	return xQueueReceive(PressureSensor_Queue, (void*)&limitForce, 0);
}
