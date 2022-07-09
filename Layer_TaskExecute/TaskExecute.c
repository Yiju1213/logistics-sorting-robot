
/* Includes */
#include "TaskExecute.h"



/* Defines */
#define KEY1_CLICKED    (KeyValue & 0x01)
#define KEY2_CLICKED    (KeyValue & 0x02)
#define KEY3_CLICKED    (KeyValue & 0x04)
#define KEY4_CLICKED    (KeyValue & 0x08)

#define UltraSonicTim		TIM12
#define UltraSonicHtim		htim12
#define UltraSonicChannel 	TIM_CHANNEL_2

#define ZEROS_VEL 1600
#define STOP 0


/* Extern Members */
extern QueueHandle_t Serial_Queue;
extern QueueHandle_t JointDrive_Queue;
extern QueueHandle_t Coordinate_Queue;
extern osSemaphoreId SYNC_SignalHandle;
extern osSemaphoreId CameraOpen_SigalHandle;
extern QueueHandle_t PressureSensor_Queue;
extern QueueHandle_t RangingSensor_Queue;

/* Object */
UltraSonic_t *pUltraSonic;
WeightSensor_t *pWeightSensor;

/* Tasks */
void JointTaskFunc(void const * argument)
{
	BaseType_t xReturn;
	uint8_t pack[5];
	uint8_t jointName;
	int16_t tarPos, tarVel;
	StepperModuleInit();
	SwitchModuleInit(	&SW1Clicked,	&SW2Clicked,	&SW3Clicked,	&SW4Clicked, &SW5Clicked, &SW6Clicked,
						&SW1Released,	&SW2Released,	&SW3Released,	&SW4Released, &SW5Released, &SW6Released);
	for(;;)
	{
		xReturn = xQueueReceive(JointDrive_Queue,(void*)&pack,portMAX_DELAY);
		if(pdPASS == xReturn)
		{
			// 拆包
			jointName = pack[0];
			// 关节复位
			if(jointName == ALL_JOINT_RESET)
				JointAllResetToHead();
			tarPos = ((pack[1]<<8) | pack[2]);
			tarVel = ((pack[3]<<8) | pack[4]);
			DriveJointMove(jointName, tarPos, tarVel);	
		}
	}
}

void RangingTaskFunc(void const * argument)
{
	float curLen, limitLen;
	pUltraSonic = Allocate_UltraSonic(TRIG_GPIO_Port,TRIG_Pin,UltraSonicTim, &UltraSonicHtim,UltraSonicChannel);
	for(;;)
	{
		if(pUltraSonic != NULL /** 有其他的也可加上 */)
		{
			// 等待工作信号（队列形式，将给出限值，对于多对象情况，可以在队列消息追加对应对象）
			xQueueReceive(RangingSensor_Queue, (void*)&limitLen, 0);
			UltraSonic_Actuation(pUltraSonic);
			curLen = pUltraSonic->Len;
			if(curLen > limitLen)
			{
				// 释放信号（队列形式，不在乎内容，只关心是否发送）
				xQueueSend(RangingSensor_Queue, &limitLen, portMAX_DELAY);
				// 检测完成，置最大值，则不会再进入这一条件
				limitLen = 0x7FFFFFFF;
			}
		}
		osDelay(100);
	}
}

void SerialTaskFunc(void const * argument)
{
	BaseType_t xReturn;
	SerialPackge_t pack;
	int16_t data[SERIAL_DATABYTE/2];
	SerialInit(NULL);
	for(;;)
	{
		xReturn = xQueueReceive(Serial_Queue,(void*)&pack,portMAX_DELAY);
		if(pdPASS == xReturn)
		{
			// 包数据提取,提取到的数据放入data
			SerialPackExtract(&pack, data);
			int16_t interaction = data[0];
			switch (interaction)
			{
				// 上位机SYNC同步信号
				case GET_SYNC:
					// 释放已同步信号量
					xSemaphoreGive(SYNC_SignalHandle);
					break;
				// 摄像头已开启信号
				case CAM_OPENED:
					// 释放摄像头开启信号量
					xSemaphoreGive(CameraOpen_SigalHandle);
					break;
				// 获取到坐标信息
				case MESSAGE_GET:
					// 将坐标信息放入COOR_QUEUE传给上层进行相关逆解
					xQueueSend(Coordinate_Queue,(void*)(&data[1]), portMAX_DELAY);
					break;
			}
		}
	}
}

void WeighingTaskFunc(void const * argument)
{
	uint8_t i;
	int32_t zeroForce, curForce, limitForce;
	pWeightSensor = WeighingSensor_Allocate(HX711SCK_GPIO_Port, HX711SCK_Pin,
										HX711_Dout_GPIO_Port, HX711_Dout_Pin);
	// 零位重量读取
	for(i = 4; i>0;i--)
	{
		if(pWeightSensor != NULL)
		{
			WeighingSensor_Actuation(pWeightSensor);
			zeroForce = pWeightSensor->Weight;
		}
	}
	for(;;)
	{
		if(pWeightSensor != NULL /** 有其他的也可以加上 */)
		{
			// 等待工作信号（队列形式，将给出限值，对于多对象情况，可以在队列消息追加对应对象）
			xQueueReceive(PressureSensor_Queue, (void*)&limitForce, 0);
			
			WeighingSensor_Actuation(pWeightSensor);
			curForce = pWeightSensor->Weight - zeroForce;
			if(curForce > limitForce)
			{
				// 释放信号（队列形式，不在乎内容，只关心是否发送）
				xQueueSend(PressureSensor_Queue,&limitForce, portMAX_DELAY);
				// 检测完成，置最大值，则在下次设置前不会再进入这一条件
				limitForce = 0x7FFFFFFF;
			}
			osDelay(100); 
		}
	}
}

void JointAllResetToHead(void)
{
	uint8_t jointID;
	for(jointID = 0; jointID < 4; jointID++)
	{
		JointResetToHead(jointID);
	}
}
