/**
  ******************************************************************************
  * @file        : Switch.c
  * @author      : CQS
  * @brief       : 接近开关中断处理
  ******************************************************************************
  * @attention   针对接近开关的接通瞬间和断开瞬间提供了回调函数
                 由于硬件问题，行程开关触发的时候一定会有抖动，进而造成多次触发中断函数
                 所以需要对其进行处理，最终实现按键触发一次就调用一次函数的效果  
  ******************************************************************************
*/
#include "Switch.h"
#include "main.h"

static void (*pUserSW1Clicked)(void) = NULL;
static void (*pUserSW2Clicked)(void) = NULL;
static void (*pUserSW3Clicked)(void) = NULL;
static void (*pUserSW4Clicked)(void) = NULL;
static void (*pUserSW5Clicked)(void) = NULL;
static void (*pUserSW6Clicked)(void) = NULL;

static void (*pUserSW1Released)(void) = NULL;
static void (*pUserSW2Released)(void) = NULL;
static void (*pUserSW3Released)(void) = NULL;
static void (*pUserSW4Released)(void) = NULL;
static void (*pUserSW5Released)(void) = NULL;
static void (*pUserSW6Released)(void) = NULL;

/**
  * @brief 行程开关模块初始化函数
  * @param SW1回调函数指针
  * @param SW2回调函数指针
  * @param SW3回调函数指针
  * @param SW4回调函数指针
  * @param SW5回调函数指针
  * @param SW6回调函数指针
*/
void SwitchModuleInit(	void (*pSW1ClickedFunc)(void), void (*pSW2ClickedFunc)(void), 
						void (*pSW3ClickedFunc)(void), void (*pSW4ClickedFunc)(void),
						void (*pSW5ClickedFunc)(void), void (*pSW6ClickedFunc)(void),
						void (*pSW1ReleasedFunc)(void), void (*pSW2ReleasedFunc)(void),
						void (*pSW3ReleasedFunc)(void), void (*pSW4ReleasedFunc)(void),
						void (*pSW5ReleasedFunc)(void), void (*pSW6ReleasedFunc)(void))
{							
    pUserSW1Clicked = pSW1ClickedFunc;
    pUserSW2Clicked = pSW2ClickedFunc;
    pUserSW3Clicked = pSW3ClickedFunc;
    pUserSW4Clicked = pSW4ClickedFunc;
	pUserSW5Clicked = pSW5ClickedFunc;
    pUserSW6Clicked = pSW6ClickedFunc;
	
    pUserSW1Released = pSW1ReleasedFunc;
    pUserSW2Released = pSW2ReleasedFunc;
    pUserSW3Released = pSW3ReleasedFunc;
    pUserSW4Released = pSW4ReleasedFunc;
	pUserSW5Released = pSW5ReleasedFunc;
    pUserSW6Released = pSW6ReleasedFunc;
}

/** @brief 外部中断1服务函数 */
void EXTI1_IRQHandler(void){
	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_1) != RESET){
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
        HAL_Delay(10);      // 消抖
        if(HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET){
            if(pUserSW1Released != NULL){
                (*pUserSW1Released)();
            }
        }else{
            if(pUserSW1Clicked != NULL){
                (*pUserSW1Clicked)();
            }
        }
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
        while(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_1) == SET);
	}
}

/** @brief 外部中断2服务函数 */
void EXTI2_IRQHandler(void){
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_2) != RESET){
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);
        HAL_Delay(10);
        if(HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == GPIO_PIN_RESET){
            if(pUserSW2Released != NULL){
				
                (*pUserSW2Released)();
            }
        }else{
            if(pUserSW2Clicked != NULL){
				
                (*pUserSW2Clicked)();
            }
        }
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);
        while(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_2) == SET);
    }
}

/** @brief 外部中断3服务函数 */
void EXTI3_IRQHandler(void){
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_3) != RESET){
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);
        HAL_Delay(10);
        if(HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin) == GPIO_PIN_RESET){
            if(pUserSW3Released != NULL){
                (*pUserSW3Released)();
            }
        }else{
            if(pUserSW3Clicked != NULL){
                (*pUserSW3Clicked)();
            }
        }
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3); 
        while(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_3) == SET);
    }
}

/** @brief 外部中断4服务函数 */
void EXTI4_IRQHandler(void){
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4) != RESET){
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
        HAL_Delay(10);
        if(HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin) == GPIO_PIN_RESET){
            if(pUserSW4Released != NULL){
                (*pUserSW4Released)();
            }
        }else{
            if(pUserSW4Clicked != NULL){ 
                (*pUserSW4Clicked)();
            }
        }
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
        while(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4) == SET);
    }
}

/** @brief 外部中断5-9服务函数（仅用9） */
void EXTI9_5_IRQHandler(void)
{
	if(__HAL_GPIO_EXTI_GET_IT(SW5_Pin) != RESET){
        __HAL_GPIO_EXTI_CLEAR_IT(SW5_Pin);
        HAL_Delay(10);
        if(HAL_GPIO_ReadPin(SW5_GPIO_Port, SW5_Pin) == GPIO_PIN_RESET){
            if(pUserSW5Released != NULL){
                (*pUserSW5Released)();
            }
        }else{
            if(pUserSW5Clicked != NULL){ 
                (*pUserSW5Clicked)();
            }
        }
        __HAL_GPIO_EXTI_CLEAR_IT(SW5_Pin);
        while(__HAL_GPIO_EXTI_GET_IT(SW5_Pin) == SET);
    }
}
/** @brief 外部中断10-15服务函数（仅用10） */
void EXTI15_10_IRQHandler(void)
{
	if(__HAL_GPIO_EXTI_GET_IT(SW6_Pin) != RESET){
        __HAL_GPIO_EXTI_CLEAR_IT(SW6_Pin);
        HAL_Delay(10);
        if(HAL_GPIO_ReadPin(SW6_GPIO_Port, SW6_Pin) == GPIO_PIN_RESET){
            if(pUserSW5Released != NULL){
                (*pUserSW6Released)();
            }
        }else{
            if(pUserSW5Clicked != NULL){ 
                (*pUserSW6Clicked)();
            }
        }
        __HAL_GPIO_EXTI_CLEAR_IT(SW6_Pin);
        while(__HAL_GPIO_EXTI_GET_IT(SW6_Pin) == SET);
    }
}
