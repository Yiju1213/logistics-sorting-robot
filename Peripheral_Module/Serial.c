/**
  ******************************************************************************
  * @file        : UpperLogic.c
  * @author      : CQS
  * @brief       : 上位机通信逻辑
  ******************************************************************************
  * @attention   使用方法：
                 （1）必须首先调用SerialInit函数，将接收回调函数指针传入，如果不需要，请传入NULL（请勿在接收回调函数中执行耗时操作！）
                 （2）如果需要使用串口发送数据，调用SerialPrintf函数，其用法和C语言中的printf是一样的
                 （3）当串口接收到数据时，将会自动调用传入的回调函数，用户只需要在自己的回调函数里面编写相应的逻辑即可
  ******************************************************************************
*/
#include "Serial.h"
#include "LEDModule.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "usart.h"
#include "cmsis_os.h"

#define TX_BUFFER_SIZE		256
#define RX_BUFFER_SIZE		256
#define UPPER_UART			(&huart1)
#define FRAMESTART			0xFF
#define FRAMEEND				0xAA

/* External Members */
extern QueueHandle_t Serial_Queue;
/* Internal Members */
static SerialPackge_t Pack;
static uint8_t UartTxBuffer[TX_BUFFER_SIZE];  // 串口发送缓冲区
static uint8_t UartRxBuffer[RX_BUFFER_SIZE];  // 串口接收缓冲区
static void onUartReceivedData(UART_HandleTypeDef *huart, uint16_t Size);
/**
  * @brief 串口初始化函数
  * @param 回调函数指针
*/
void SerialInit(void (*pFunc)(uint8_t *, uint32_t)){ 
	// 登记接收回调函数->传入串口句柄以及回调函数
    while(HAL_UART_RegisterRxEventCallback(UPPER_UART, onUartReceivedData) == HAL_ERROR);
	// 配置并开启空闲DMA接收
    HAL_UARTEx_ReceiveToIdle_DMA(UPPER_UART, UartRxBuffer, RX_BUFFER_SIZE);
}

/**
  * @brief 串口发送函数，用法和Printf是一样的
  * @param 要发送的东西
*/
void SerialPrintf(const char *Format, ...){
    va_list pA;
    uint32_t Length;
	HAL_StatusTypeDef HAL_State;
    
    va_start(pA, Format);
    Length = vsnprintf((char*)UartTxBuffer, TX_BUFFER_SIZE, (char*)Format, pA);
    va_end(pA);
    
    HAL_State = HAL_UART_Transmit_DMA(UPPER_UART, UartTxBuffer, Length);
	if(HAL_State != HAL_OK)	AlarmLED();
}

/** @brief 空闲中断回调函数 */
static void onUartReceivedData(UART_HandleTypeDef *huart, uint16_t Size){
	// 确认是上位机对应串口
	if(huart == UPPER_UART)
	{
		BaseType_t pxHigherPriorityTaskWoken;
		// 改变DATABYTE即可修改队列大小
		xQueueSendFromISR(Serial_Queue,(void*)&UartRxBuffer,&pxHigherPriorityTaskWoken);
		// 重启空闲接收
		HAL_UARTEx_ReceiveToIdle_DMA(huart, UartRxBuffer, RX_BUFFER_SIZE);
	}
}

void SerialPackTransmit(uint8_t* pData)
{
	uint8_t byte;
	/* 数据帧封装 */
	Pack.FrameStart = FRAMESTART;
	Pack.DataByte 	= SERIAL_DATABYTE;
	for(byte = 0; byte < SERIAL_DATABYTE; byte ++)
		Pack.Data[byte] = pData[byte];
	Pack.FrameEnd 	= FRAMEEND;
	/* 数据帧传输 */
	while(__HAL_UART_GET_FLAG(UPPER_UART,UART_FLAG_TC) == RESET)	// 等待硬件上一次传输完成
	{
		SetLEDOnorOff(3,1);
	}
	HAL_UART_Transmit_DMA(UPPER_UART, (const uint8_t*)&Pack, sizeof(Pack));
}

void SerialPackExtract(uint8_t *pPack, uint8_t *pData)
{
	uint8_t byte;
	// 帧起始校验
	if(pPack[0] == FRAMESTART)
	{
		// 帧数据长度校验
		if(pPack[1] == SERIAL_DATABYTE)
		{
			// 帧数据接收
			for(byte = 0; byte<SERIAL_DATABYTE ; byte++)
			{
				pData[byte] = pPack[2+byte]; 
			}
		}
		// 帧结尾校验
		if(pPack[2+SERIAL_DATABYTE] == FRAMEEND)	
		{
			return;
		}
	}
	AlarmLED();
}
