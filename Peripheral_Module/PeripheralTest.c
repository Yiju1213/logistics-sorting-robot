/* Includes */
#include "PeripheralTest.h"
#include "Serial.h"
#include "ULtraSonic.h"
#include "Stepper.h"
#include "Switch.h"

/* Defines */
#define ABS(x)   ((x) > 0 ? (1) : (-1))
#define M42REVPULSE	6400
#define M42MAXVELRATIO 3
#define ZAXISSTEP 4
#define SW1 0
#define SW2 1
#define SW3 2
#define SW4 3
#define M1Head SW1
#define M1Tail SW2

#define M1	1
#define M2	2
#define M3	3
#define M4	4
#define KEY1_CLICKED    (KeyValue & 0x01)
#define KEY2_CLICKED    (KeyValue & 0x02)
#define KEY3_CLICKED    (KeyValue & 0x04)
#define KEY4_CLICKED    (KeyValue & 0x08)
#define ZEROS_VEL 1600
#define M42MMTOPULSE (M42REVPULSE / ZAXISSTEP)


/* Extern Members */
extern QueueHandle_t Serial_Queue;
extern QueueHandle_t Stepper_Queue;
/* Object */
UltraSonic_t *pUltraSonic;
Switch_t Switch[4];
/* Tasks */
void TestTaskFunc(void const * argument)
{
	BaseType_t xReturn;
	
	uint8_t data[SERIAL_DATABYTE];
	uint8_t stepperID;
	int16_t tarPos_mm;
	StepperModuleInit();
	SwitchModuleInit(	SW1Clicked,	SW2Clicked,	SW3Clicked,	SW4Clicked,
						SW1Released,SW2Released,SW3Released,SW4Released	   );
	
	for(;;)
	{
		xReturn = xQueueReceive(Stepper_Queue,(void*)&data,portMAX_DELAY);
		if(pdPASS == xReturn)
		{
			stepperID = data[0];
			// 最后要转成有符号整型
			tarPos_mm = (int16_t)((data[1] << 8) | data[2]);
			SetStepperTarPosWithVel(stepperID, (tarPos_mm * M42MMTOPULSE), M42REVPULSE);
			// 回传，调试用
			SerialPackTransmit(data);
			ToggleLED(2);
		}
		
	}
}
void RangingTaskFunc(void const * argument)
{
	uint8_t KeyValue;
	//pUltraSonic = Allocate_UltraSonic(TRIG_GPIO_Port,TRIG_Pin,UltraSonicTim, &UltraSonicHtim,UltraSonicChannel);
	for(;;)
	{
		KeyValue = KeyScan();
		if(KEY1_CLICKED)
		{
			// 注意硬件初始化顺序
			StepperGoSwitchZeros(M1, M1Head);
		}
		if(KEY2_CLICKED)
		{
			// 注意硬件初始化顺序
			StepperGoSwitchZeros(M1, M1Tail);
		}
		// 等到检测允许信号（如果有多个，就对应多个允许信号）
		// waitSignal
		// 开始检测
		//UltraSonic_Actuation(pUltraSonic);
		//SerialPrintf("buf1 = %d\tbuf2 = %d\t len = %.2f\n",pUltraSonic->Data[0], pUltraSonic->Data[1],pUltraSonic->Len);
		osDelay(100);
	}
}

void SerialTaskFunc(void const * argument)
{
	BaseType_t xReturn;
	SerialPackge_t pack;
	uint8_t data[SERIAL_DATABYTE];
	SerialInit(NULL);
	for(;;)
	{
		xReturn = xQueueReceive(Serial_Queue,(void*)&pack,portMAX_DELAY);
		if(pdPASS == xReturn)
		{
			// 包数据提取
			SerialPackExtract((uint8_t *)&pack, data);
			/* 判断是否为电机控制信息！！！ */
			// （如果是）将控制信息发到控制任务
			xQueueSend(Stepper_Queue,(void*)data, portMAX_DELAY);
		}
	}
}

/* Functions */
void StepperGoSwitchZeros(uint8_t MotorID, uint8_t SwID)
{
	int8_t tarDir;
	switch (SwID)
	{
		case M1Head:
			tarDir = -1;
			Switch[M1Head].mode = ZerosP1;
			break;
		case M1Tail:
			tarDir = 1;
			Switch[M1Tail].mode = ZerosP1;
			break;
	}
	SetStepperTarVel(MotorID, tarDir * M42REVPULSE * M42MAXVELRATIO);
}
/** @Tips 添加行程开关与电机的绑定工作？ */
void SW1Clicked(void)
{
	// SetLEDOnorOff(1,1);
	switch(Switch[SW1].mode)
	{
		case Limit:	
			SetStepperTarVel(1,0);		
			break;
		case ZerosP1:
			SetStepperTarVel(1, -ABS(GetStepperDir(1))*3200);
			break;
		case ZerosP2:
			SetStepperZeroPos(1);
			// if(GetStepperCurPos(1) == 0)	SetLEDOnorOff(2,1);
			break;
	}
}
void SW2Clicked(void)
{
	
}
void SW3Clicked(void)
{
	
}
void SW4Clicked(void)
{
	
}
void SW1Released(void)
{
	switch(Switch[SW1].mode)
	{
		case ZerosP1:	
			SetStepperTarVel(1, -ABS(GetStepperDir(1))*1600);
			Switch[SW1].mode = ZerosP2;
			break;
		default:
			Switch[SW1].mode = Limit;
	}
}
void SW2Released(void)
{
	
}
void SW3Released(void)
{
	
}
void SW4Released(void)
{
	
}


