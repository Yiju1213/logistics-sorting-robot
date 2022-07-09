/**
  ******************************************************************************
  * @file        : WeighingSensor.c
  * @author      : DB & WYR
  * @brief       : 压力测重模块逻辑
  ******************************************************************************
  * @attention   使用方法：
					1. 指定prvWeightIndex
					3. 在外部调用测重模块申请函数WeighingSensor_Allocate，传入对象信息，获得对象指针
					4. 循环中调用WeighingSensor_Actuation可实现周期性测距
  ******************************************************************************
*/
#include "WeighingSensor.h"

#define DATASCALE 		106.5
#define DATABIT			24
#define WEIGHTSENSOR_NUM 1

static WeightSensor_t prvWeightSensor[WEIGHTSENSOR_NUM];
static uint8_t prvWeightIndex = 0;

/* Functions */
/** @brife 压力测重模块对象申请 */
WeightSensor_t* WeighingSensor_Allocate(GPIO_TypeDef* sck_port, uint32_t sck_pin,
									GPIO_TypeDef* dout_port, uint32_t dout_pin)
{
	uint8_t id;
	WeightSensor_t* pSpecific;
	// 最大申请数判别
	id = prvWeightIndex;
	if(id >= WEIGHTSENSOR_NUM) return NULL;
	// 开始申请对象
	pSpecific = &prvWeightSensor[id];
	prvWeightIndex ++;
	// 填入对象属性
	pSpecific->SCK_Port	= sck_port;
	pSpecific->SCK_Pin	= sck_pin;
	pSpecific->Dout_Port	= dout_port;
	pSpecific->Dout_Pin	= dout_pin;
	// 返回实例对象指针
	return pSpecific;
}
/** @brife 压力测重模块测距任务（任务发起及结果计算） */
void WeighingSensor_Actuation(WeightSensor_t* This)
{
	uint8_t cnt = 0;
	int32_t data = 0;
	// 在HX711Dout引脚输出高电平，即未准备输出数据时，SCK保持低电平
	HAL_GPIO_WritePin(This->Dout_Port,This->Dout_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(This->SCK_Port,This->SCK_Pin,GPIO_PIN_RESET);
	delay_us(1);
	// 等待Dout引脚输出低电平，即准备开始发送数据
	while(HAL_GPIO_ReadPin(This->Dout_Port, This->Dout_Pin) != GPIO_PIN_RESET);
	// Dout引脚输出低电平时，数据随着SCK脉冲时钟沿开始发送
	for(cnt=0; cnt<DATABIT ;cnt++)
	{
		// SCK写入高电平1us
		HAL_GPIO_WritePin(This->SCK_Port,This->SCK_Pin,GPIO_PIN_SET);
		delay_us(1);
		data = data << 1;
		// SCK写入低电平，同时读出1bit Dout数据（最后保证低电平持续时间1us）
		HAL_GPIO_WritePin(This->SCK_Port,This->SCK_Pin,GPIO_PIN_RESET);
		if(HAL_GPIO_ReadPin(This->Dout_Port, This->Dout_Pin) == GPIO_PIN_SET)
		{
			data++;
		}
			
		// 等待下一bit Dout数据
		
		delay_us(1);
	}
	// 数据脉冲走完，走通道脉冲（128增益A通道走1个脉冲）
	HAL_GPIO_WritePin(This->SCK_Port,This->SCK_Pin,GPIO_PIN_SET);
	delay_us(1);
	HAL_GPIO_WritePin(This->SCK_Port,This->SCK_Pin,GPIO_PIN_RESET);
	// 逻辑左移8位右端全补0，逻辑右移8位左端按照符号位补位，完成有符号24bitAD到有符号32bit转换
	data = ((data << 8) >> 8);
	// 数据更新
	This->Weight = data;
}

