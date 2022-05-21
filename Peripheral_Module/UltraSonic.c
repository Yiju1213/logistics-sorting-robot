/**
  ******************************************************************************
  * @file        : UltraSonic.c
  * @author      : WYR
  * @brief       : 超声波测距逻辑
  ******************************************************************************
  * @attention   使用方法：
					1. 指定UltraSonicNum
					2. 在UltraSonic.h头文件中定义各个超声波模块的定时器/句柄/通道
					3. 在外部调用超声波模块申请函数Allocate_UltraSonic，传入对象信息，获得对象指针
					4. 循环中调用UltraSonic_Actuation可实现周期性测距
				注意：CubeMx中定时器要开通输入捕获模式，频率要设置成1MHz（计数周期1us）
  ******************************************************************************
*/

/* Includes ------------------------ */
#include "UltraSonic.h"

/* Defines ------------------------- */
#define UltraSonicNum 1		// 超声波模块总数
/* Extern Members ------------------ */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);	// 重写__weak函数
/* Private Members ----------------- */
static UltraSonic_t prvUltraSonic[UltraSonicNum];			// 超声波对象结构体
static uint8_t prvUltraSonicIndex = 0;						// 超声波申请数索引

/* Functions ----------------------- */
/** @brife 超声波模块申请 */
UltraSonic_t* Allocate_UltraSonic(GPIO_TypeDef *trigPort, uint32_t trigPin, TIM_TypeDef *tim, TIM_HandleTypeDef *htim, uint32_t channel)
{
	UltraSonic_t *pSpecific;
	uint8_t ID;
	ID = prvUltraSonicIndex;
	if(ID >= UltraSonicNum)	return NULL; // 超过可申请最大数
	// 开始申请对象
	pSpecific = &prvUltraSonic[prvUltraSonicIndex];
	prvUltraSonicIndex ++;
	// 填入对象属性
	pSpecific->TrigPort = trigPort;
	pSpecific->TrigPin	= trigPin;
	pSpecific->Tim		= tim;
	pSpecific->Htim		= htim;
	pSpecific->Channel	= channel;
	// 回传对象指针
	return pSpecific;
}

/** @brife 超声波模块测距任务（任务发起及结果计算） */
void UltraSonic_Actuation(UltraSonic_t *This)
{
	// 1. 判断是否处于空闲状态
	if(This->State == TRIG_WAIT)
	{
		// 1.1 给trig 1ms高电平信号
		HAL_GPIO_WritePin(This->TrigPort,This->TrigPin,GPIO_PIN_SET);
		osDelay(1);
		HAL_GPIO_WritePin(This->TrigPort,This->TrigPin,GPIO_PIN_RESET);
		// 1.2 设置上升沿捕获
		__HAL_TIM_SET_CAPTUREPOLARITY(This->Htim,This->Channel,TIM_INPUTCHANNELPOLARITY_RISING);
		// 1.3 启动输入捕获
		HAL_TIM_IC_Start_IT(This->Htim,This->Channel);
		// 1.4 更新状态
		This->State = ECHO_RISING;
	}
	// 2. 判断是否处于捕获结束状态
	if(This->State == OVER)
	{
		// 2.1 计算检测结果
		// 计算高电平时间差值代表时间维持的长度，因为定时器设置1us自加一次，所以时长单位为us
		// 乘以0.017的是因为声波速度为340m/s，而1s=1000000us，且计数一次距离是一个来回
		// 要除以2，此处单位为cm
		This->Len = (float)(This->Data[1] - This->Data[0]) * 0.017;
		// 2.2 刷新状态，使可以进行下一次触发电平
		HAL_TIM_IC_Stop_IT(This->Htim,This->Channel);
		This->State = TRIG_WAIT;
	}
}

/** @brife 超声波模块测距任务（中断捕获） */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	uint8_t i;
	UltraSonic_t *This;
	// 找到多个超声波模块中具体进入捕获中断的
	for(i=0;i<UltraSonicNum;i++)
	{
		This = &prvUltraSonic[i];
		if(This->Tim == htim->Instance)
		{
			// 1. 上升沿捕获状态
			if(This->State == ECHO_RISING)
			{
				// 1.1 重置CNT
				__HAL_TIM_SetCounter(This->Htim, 0);
				// 1.2 读取当前值
				This->Data[0] = __HAL_TIM_GetCounter(This->Htim);
				// 1.3 设置下一个捕获为下降沿
				__HAL_TIM_SET_CAPTUREPOLARITY(This->Htim,This->Channel,TIM_INPUTCHANNELPOLARITY_FALLING);
				// 1.4 设置状态
				This->State = ECHO_FALLING;
			}
			// 2. 下降沿捕获状态
			else if(This->State == ECHO_FALLING)
			{
				// 2.1 读取当前值
				This->Data[1] = __HAL_TIM_GetCounter(This->Htim);
				// 2.2 设置状态
				This->State = OVER;
			}
		}
		//break;
	}
}
