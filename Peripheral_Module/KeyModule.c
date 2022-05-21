/**
  ******************************************************************************
  * @file        : KeyMoudle.c
  * @author      : CQS
  * @brief       : 按键模块
  ******************************************************************************
  * @attention   uint8_t KeyScan(void)  // 返回值低四位指示了按键是否被按下（只会指示一次，即长按和短按效果是一样的）
  ******************************************************************************
 */
#include "KeyModule.h"
#include "main.h"

static uint8_t prvButtonClickedFlag[4] = {0, 0, 0, 0};

/** @brief 按键扫描函数 */
uint8_t KeyScan(void)
{
    uint8_t rValue = 0;
    // KEY1
    if(HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
	{
        if(prvButtonClickedFlag[0] == 0)
		{
            rValue |= 0x01;
            prvButtonClickedFlag[0] = 1;
        }
    }	
	else
	{
		prvButtonClickedFlag[0] = 0;
	}
    
    // KEY2
    if(HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET){
        if(prvButtonClickedFlag[1] == 0){
            rValue |= 0x02;
            prvButtonClickedFlag[1] = 1;
        }
    }else{
        prvButtonClickedFlag[1] = 0;
    }
    
    // KEY3
    if(HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin) == GPIO_PIN_RESET){
        if(prvButtonClickedFlag[2] == 0){
            rValue |= 0x04;
            prvButtonClickedFlag[2] = 1;
        }
    }else{
        prvButtonClickedFlag[2] = 0;
    }
    
    // KEY4
    if(HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin) == GPIO_PIN_RESET){
        if(prvButtonClickedFlag[3] == 0){
            rValue |= 0x08;
            prvButtonClickedFlag[3] = 1;
        }
    }	else{
        prvButtonClickedFlag[3] = 0;
    }
    
    return rValue;
}
