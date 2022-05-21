/**
  ******************************************************************************
  * @file        : LEDMoudle.c
  * @author      : CQS
  * @brief       : LED模块
  ******************************************************************************
  * @attention   直接调用对应函数即可
  ******************************************************************************
 */
#include "LEDModule.h"
#include "main.h"

/**
  * @brief  设定LED灯亮灭
  * @param  LEDID:1~4
            Enabled:0关1开
*/
void SetLEDOnorOff(uint8_t LEDID, LEDStatus_t status){
    switch(LEDID){
        case LED1:(status == TRUE ? HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET) : HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET));break;
        case LED2:(status == TRUE ? HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET) : HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET));break;
        case LED3:(status == TRUE ? HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET) : HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET));break;
        case LED4:(status == TRUE ? HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET) : HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET));break;
        default:break;
    }
}

/**
  * @brief  翻转LED灯电平
  * @param  LEDID:1~4
*/
void ToggleLED(uint8_t LEDID){
    switch(LEDID){
        case LED1:HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);break;
        case LED2:HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);break;
        case LED3:HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);break;
        case LED4:HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);break;
        default:break;
    }
}

void AlarmLED(void)
{
	SetLEDOnorOff(ALARMLED, LEDON);
}