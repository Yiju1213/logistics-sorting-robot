/**
  ******************************************************************************
  * @file        : Switch.h
  * @author      : CQS
  * @brief       : 接近开关中断处理
  ******************************************************************************
  * @attention   
  ******************************************************************************
*/
#ifndef SWITCH_H
#define SWITCH_H
#include "stdint.h"
/* Enum */
typedef enum
{
	Limit = 0,
	ZerosP1,
	ZerosP2
}SwitchMode;
/* Struct */
typedef struct
{
	SwitchMode mode;
}Switch_t;


void SwitchModuleInit(	void (*pSW1ClickedFunc)(void), void (*pSW2ClickedFunc)(void), 
						void (*pSW3ClickedFunc)(void), void (*pSW4ClickedFunc)(void),
						void (*pSW5ClickedFunc)(void), void (*pSW6ClickedFunc)(void),
						void (*pSW1ReleasedFunc)(void), void (*pSW2ReleasedFunc)(void),
						void (*pSW3ReleasedFunc)(void), void (*pSW4ReleasedFunc)(void),
						void (*pSW5ReleasedFunc)(void), void (*pSW6ReleasedFunc)(void));

#endif
