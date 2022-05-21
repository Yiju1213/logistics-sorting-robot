/**
  ******************************************************************************
  * @file        : LEDMoudle.h
  * @author      : CQS
  * @brief       : LED模块头文件
  ******************************************************************************
  * @attention
  ******************************************************************************
 */
#ifndef LEDMOUDLE_H
#define LEDMOUDLE_H
/* Includes ----------------------- */
#include "stdint.h"

/* Defines ------------------------ */
#define LED1 1
#define LED2 2
#define LED3 3
#define LED4 4
#define ALARMLED LED1
#define LEDOFF	0
#define LEDON	1
/* Struct ------------------------- */
typedef enum
{
	Off = 0,
	On
}LEDStatus_t;


void SetLEDOnorOff(uint8_t LEDID, LEDStatus_t status);
void ToggleLED(uint8_t LEDID);
void AlarmLED(void);
#endif
