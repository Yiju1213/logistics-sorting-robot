/**
  ******************************************************************************
  * @file        : Stepper.c
  * @author      : CQS
  * @brief       : 开发板M1 M2 M3的控制逻辑
  ******************************************************************************
  * @attention  使用方法：
                （1）在Setup函数中调用StepperMoudleInit函数，使得中断函数生效
                （2）调用设置位置函数等，实现电机的运动
  ******************************************************************************
*/
#include "Stepper.h"
#include "tim.h"

#define SIGNAL(x)   ((x) > 0 ? (1) : (-1))
#define TRUE        1
#define FALSE       0
#define FTIM_D_PSC	1000000.f
static StepperCtx_t StepperArray[] = {{nENBL_M1_GPIO_Port, nENBL_M1_Pin, DIR_M1_GPIO_Port, DIR_M1_Pin, &htim8, TIM_CHANNEL_4, STEPPERIDLE, 0, 0, 0, FALSE, 1}, 
                                      {nENBL_M2_GPIO_Port, nENBL_M2_Pin, DIR_M2_GPIO_Port, DIR_M2_Pin, &htim3, TIM_CHANNEL_1, STEPPERIDLE, 0, 0, 0, FALSE, 1}, 
                                      {nENBL_M3_GPIO_Port, nENBL_M3_Pin, DIR_M3_GPIO_Port, DIR_M3_Pin, &htim4, TIM_CHANNEL_2, STEPPERIDLE, 0, 0, 0, FALSE, 1}};
static void prvTIMPeriodElapsedCallback(TIM_HandleTypeDef *htim);

/** @brief  步进电机模块初始化函数 */
HAL_StatusTypeDef StepperModuleInit(void){
    HAL_StatusTypeDef rValue = HAL_OK;
    for(uint8_t i = 0;i < 3;i++){
        rValue |= HAL_TIM_RegisterCallback(StepperArray[i].pTIM, HAL_TIM_PERIOD_ELAPSED_CB_ID, prvTIMPeriodElapsedCallback);
    }
    return rValue;
}

/**
  * @brief  获取当前步进电机位置
  * @param  MotorID:    电机ID[1 2 3]
  * @retval  设置结果
*/
int32_t GetStepperCurPos(uint8_t MotorID){
    return StepperArray[MotorID - 1].CurPos;
}
/**
  * @brief  获取当前步进电机速度
  * @param  MotorID:    电机ID[1 2 3]
  * @retval  设置结果
*/
int32_t GetStepperCurVel(uint8_t MotorID)
{
	return StepperArray[MotorID - 1].CurVel;
}
/**
  * @brief  获取当前步进电机模式
  * @param  MotorID:    电机ID[1 2 3]
  * @retval  当前模式
*/
StepperMode_t GetStepperCurMode(uint8_t MotorID){
    return StepperArray[MotorID - 1].Mode;
}
/**
  * @brief  获取当前步进电机转向
  * @param  MotorID:    电机ID[1 2 3]
  * @retval  1：正转  -1：反转  0：停止
*/
int8_t 	GetStepperDir(uint8_t MotorID)
{
	return StepperArray[MotorID - 1].Dir;
}
/**
  * @brief  步进电机目标位置设置
  * @param  MotorID:    电机ID[1 2 3]
            TarPos:     电机目标位置[pulses]
  * @retval  设置结果
*/
HAL_StatusTypeDef SetStepperTarPos(uint8_t MotorID, int32_t TarPos){
    HAL_StatusTypeDef rValue = HAL_OK;
    SetStepperEnabled(MotorID, TRUE);
    StepperArray[MotorID - 1].Enabled = TRUE;
    
    StepperArray[MotorID - 1].TarPos = TarPos;
    if(TarPos != StepperArray[MotorID - 1].CurPos){ 
        if(TarPos > StepperArray[MotorID - 1].CurPos){
            HAL_GPIO_WritePin(StepperArray[MotorID - 1].DIRGPIOx, StepperArray[MotorID - 1].DIRGPIO_Pin, GPIO_PIN_SET);
            StepperArray[MotorID - 1].Dir = 1;
        }  
        else if(TarPos < StepperArray[MotorID - 1].CurPos){
            HAL_GPIO_WritePin(StepperArray[MotorID - 1].DIRGPIOx, StepperArray[MotorID - 1].DIRGPIO_Pin, GPIO_PIN_RESET);
            StepperArray[MotorID - 1].Dir = -1;
        }
        rValue |= HAL_TIM_PWM_Start(StepperArray[MotorID - 1].pTIM, StepperArray[MotorID - 1].TIMChannel);
        rValue |= HAL_TIM_Base_Start_IT(StepperArray[MotorID - 1].pTIM);
        StepperArray[MotorID - 1].Mode = POSMODE;
    }
    return rValue;
}

/**
  * @brief  步进电机目标位置设置，全程将按照设定目标速度运行
  * @param  MotorID:    电机ID[1 2 3]
            TarPos:     电机目标位置[pulses]
            TarVel:     电机目标速度[pulses / s](只会取绝对值)
  * @retval 设置结果
*/
HAL_StatusTypeDef SetStepperTarPosWithVel(uint8_t MotorID, int32_t TarPos, int32_t TarVel){
    HAL_StatusTypeDef rValue = HAL_OK;
    SetStepperEnabled(MotorID, TRUE);
    
	// 保存速度值至电机CurVel、位移值至电机TarVel
	StepperArray[MotorID - 1].CurVel = TarVel;
	StepperArray[MotorID - 1].TarPos = TarPos;
	if(TarVel == 0)
	{
		SetStepperTarVel(MotorID, 0);
		return rValue;
	}
    else if(TarPos != StepperArray[MotorID - 1].CurPos){
        // 设置目标速度
        if(TarVel < 0){
            TarVel = -TarVel;
        }
        uint16_t NewPeriodValue = 0;
		// 脉冲频率公式 Fq = F(TIM) / [(PSC + 1) * (ARR + 1)] = TarVel
		//  则有 ARR = F(TIM) / (TarVel * (PSC + 1)) - 1
		//  其中PSC在CubeMx以匹配  F(TIM)/(PSC+1) == 1M 为原则设置
		//  则有 ARR	 = (F(TIM) / (PSC + 1)) / TarVel - 1
		//  		 = 1M / TarVel - 1
		// 这样做省去了TIM硬件的判别！！！
		
		/** @Previews 未进行 F(TIM) 与 PSC 的匹配，需要进行TIM硬件判别
        if(StepperArray[MotorID - 1].pTIM->Instance == TIM3 || StepperArray[MotorID - 1].pTIM->Instance == TIM4){
            NewPeriodValue = (uint16_t)(84000000.0f / (TarVel * 84) - 1);
            __HAL_TIM_SET_AUTORELOAD(StepperArray[MotorID - 1].pTIM, NewPeriodValue);
        }else if(StepperArray[MotorID - 1].pTIM->Instance == TIM8){
            NewPeriodValue = (uint16_t)(168000000.0f / (TarVel * 168) - 1);
            __HAL_TIM_SET_AUTORELOAD(StepperArray[MotorID - 1].pTIM, NewPeriodValue);
        }
		************************************************** */
		/** @Modified 进行匹配，不需进行硬件判别 */
		// 5. 计算ARR设定值，用于调整脉冲频率
		NewPeriodValue = (uint16_t)(FTIM_D_PSC / TarVel - 1);
		// 6. 设定硬件ARR值，调整脉冲频率
		__HAL_TIM_SET_AUTORELOAD(StepperArray[MotorID - 1].pTIM, NewPeriodValue);
		// 7. 设定通道COMPARE值，用于匹配PWM波占空比50%的要求
        __HAL_TIM_SET_COMPARE(StepperArray[MotorID - 1].pTIM, StepperArray[MotorID - 1].TIMChannel, NewPeriodValue / 2);
        
        
        // 设置位置
        if(TarPos > StepperArray[MotorID - 1].CurPos){
            HAL_GPIO_WritePin(StepperArray[MotorID - 1].DIRGPIOx, StepperArray[MotorID - 1].DIRGPIO_Pin, GPIO_PIN_SET);
            StepperArray[MotorID - 1].Dir = 1;
        }  
        else if(TarPos < StepperArray[MotorID - 1].CurPos){
            HAL_GPIO_WritePin(StepperArray[MotorID - 1].DIRGPIOx, StepperArray[MotorID - 1].DIRGPIO_Pin, GPIO_PIN_RESET);
            StepperArray[MotorID - 1].Dir = -1;
        }
        rValue |= HAL_TIM_PWM_Start(StepperArray[MotorID - 1].pTIM, StepperArray[MotorID - 1].TIMChannel);
        rValue |= HAL_TIM_Base_Start_IT(StepperArray[MotorID - 1].pTIM);
        StepperArray[MotorID - 1].Mode = POSMODE;        
    }
    return rValue;
}

/**
  * @brief  步进电机目标速度设置
  * @param  MotorID:    电机ID[1 2 3]
            TarVel:     电机目标速度[pulses / s]
  * @retval 设置结果
*/
HAL_StatusTypeDef SetStepperTarVel(uint8_t MotorID, int32_t TarVel){
    HAL_StatusTypeDef rValue = HAL_OK;
    SetStepperEnabled(MotorID, TRUE);
    // 保存速度值至电机CurVel
	StepperArray[MotorID - 1].CurVel = TarVel;
    if(TarVel > 0){
        HAL_GPIO_WritePin(StepperArray[MotorID - 1].DIRGPIOx, StepperArray[MotorID - 1].DIRGPIO_Pin, GPIO_PIN_SET);
        StepperArray[MotorID - 1].Dir = 1;
    }
    else if(TarVel < 0){
        HAL_GPIO_WritePin(StepperArray[MotorID - 1].DIRGPIOx, StepperArray[MotorID - 1].DIRGPIO_Pin, GPIO_PIN_RESET);
        StepperArray[MotorID - 1].Dir = -1;
        TarVel = -TarVel;
    }
    else{
        StepperArray[MotorID - 1].Mode	= STEPPERIDLE;
		StepperArray[MotorID - 1].Dir	= 0;
        HAL_TIM_PWM_Stop(StepperArray[MotorID - 1].pTIM, StepperArray[MotorID - 1].TIMChannel);
        HAL_TIM_Base_Stop_IT(StepperArray[MotorID - 1].pTIM);
        return rValue;
    }
    
    StepperArray[MotorID - 1].Mode = VELMODE;
    
    uint16_t NewPeriodValue = 0;
    /** @Previews 未进行 F(TIM) 与 PSC 的匹配，需要进行TIM硬件判别
        if(StepperArray[MotorID - 1].pTIM->Instance == TIM3 || StepperArray[MotorID - 1].pTIM->Instance == TIM4){
            NewPeriodValue = (uint16_t)(84000000.0f / (TarVel * 84) - 1);
            __HAL_TIM_SET_AUTORELOAD(StepperArray[MotorID - 1].pTIM, NewPeriodValue);
        }else if(StepperArray[MotorID - 1].pTIM->Instance == TIM8){
            NewPeriodValue = (uint16_t)(168000000.0f / (TarVel * 168) - 1);
            __HAL_TIM_SET_AUTORELOAD(StepperArray[MotorID - 1].pTIM, NewPeriodValue);
        }
		************************************************** */
		/** @Modified 进行匹配，不需进行硬件判别 */
		// 5. 计算ARR设定值，用于调整脉冲频率
		NewPeriodValue = (uint16_t)(FTIM_D_PSC / TarVel - 1);
		// 6. 设定硬件ARR值，调整脉冲频率
		__HAL_TIM_SET_AUTORELOAD(StepperArray[MotorID - 1].pTIM, NewPeriodValue);
		// 7. 设定通道COMPARE值，用于匹配PWM波占空比50%的要求
        __HAL_TIM_SET_COMPARE(StepperArray[MotorID - 1].pTIM, StepperArray[MotorID - 1].TIMChannel, NewPeriodValue / 2);
    rValue |= HAL_TIM_PWM_Start(StepperArray[MotorID - 1].pTIM, StepperArray[MotorID - 1].TIMChannel);
    rValue |= HAL_TIM_Base_Start_IT(StepperArray[MotorID - 1].pTIM);
    return rValue;
}

/**
  * @brief  步进电机使能
  * @param  MotorID:    电机ID[1 2 3]
            Enabled:    电机使能与否[TRUE or FALSE]
  * @retval None
*/
void SetStepperEnabled(uint8_t MotorID, uint8_t Enabled){
    if(Enabled) HAL_GPIO_WritePin(StepperArray[MotorID - 1].ENBGPIOx, StepperArray[MotorID - 1].ENBGPIO_Pin, GPIO_PIN_RESET);
    else HAL_GPIO_WritePin(StepperArray[MotorID - 1].ENBGPIOx, StepperArray[MotorID - 1].ENBGPIO_Pin, GPIO_PIN_SET);
    StepperArray[MotorID - 1].Enabled = Enabled;
}

/**
  * @brief  步进电机设置当前位置为零点
  * @param  MotorID:    电机ID[1 2 3]
  * @retval None
*/
void SetStepperZeroPos(uint8_t MotorID){
    SetStepperTarVel(MotorID, 0);
    SetStepperEnabled(MotorID, TRUE);
    StepperArray[MotorID - 1].CurPos = 0;
}

/**
  * @brief  定时器回调函数
  * @param  htim:定时器句柄
  * @retval None
*/
static void prvTIMPeriodElapsedCallback(TIM_HandleTypeDef *htim){
    for(uint8_t i = 0;i < 3;i++){
        if(htim == StepperArray[i].pTIM){
            StepperArray[i].CurPos += StepperArray[i].Dir;
            if(StepperArray[i].Mode == POSMODE && StepperArray[i].CurPos == StepperArray[i].TarPos){
                HAL_TIM_PWM_Stop(StepperArray[i].pTIM, StepperArray[i].TIMChannel);
                HAL_TIM_Base_Stop_IT(StepperArray[i].pTIM);
                StepperArray[i].Mode = STEPPERIDLE;
            }
            
            // 中断中需要再判断一下是否在朝着正确的方向运行
            if(StepperArray[i].Mode == POSMODE && StepperArray[i].Dir == 1 && StepperArray[i].CurPos > StepperArray[i].TarPos){
                HAL_GPIO_WritePin(StepperArray[i].DIRGPIOx, StepperArray[i].DIRGPIO_Pin, GPIO_PIN_RESET);
                StepperArray[i].Dir = -1;
            }
            if(StepperArray[i].Mode == POSMODE && StepperArray[i].Dir == -1 && StepperArray[i].CurPos < StepperArray[i].TarPos){
                HAL_GPIO_WritePin(StepperArray[i].DIRGPIOx, StepperArray[i].DIRGPIO_Pin, GPIO_PIN_SET);
                StepperArray[i].Dir = 1;
            }
            break;
        }
    }
}
