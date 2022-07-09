/**
  ******************************************************************************
  * @file        : Stepper.h
  * @author      : CQS
  * @brief       : 开发板M1 M2 M3的控制逻辑
  ******************************************************************************
  * @attention  void StepperMoudleInit(void);                                                   // 步进电机模块初始化函数
                void SetStepperTarPos(uint8_t MotorID, int32_t TarPos);                         // 设置电机目标位置
                void SetStepperTarPosWithVel(uint8_t MotorID, int32_t TarPos, int32_t TarVel);  // 设置电机目标位置和速度
                void SetStepperTarVel(uint8_t MotorID, int32_t TarVel);                         // 设置电机目标速度
                void SetStepperEnabled(uint8_t MotorID, uint8_t Enabled);                       // 设置电机是否使能
  ******************************************************************************
 */
#ifndef STEPPER_H
#define STEPPER_H
#include "main.h"
#include "tim.h"

typedef enum{
    STEPPERIDLE = 0,
    POSMODE     = 1,
    VELMODE     = 2
}StepperMode_t;

typedef struct{
    // Enable Pin
    GPIO_TypeDef*       ENBGPIOx;
    uint16_t            ENBGPIO_Pin; 
    // Dir Pin
    GPIO_TypeDef*       DIRGPIOx;
    uint16_t            DIRGPIO_Pin; 
    // Pul Pin
    TIM_HandleTypeDef*  pTIM;
    uint32_t            TIMChannel;
    // Status
    StepperMode_t       Mode;
    int32_t             CurPos;
    int32_t             TarPos;
    int32_t             CurVel;
    uint8_t             Enabled;
    int8_t              Dir;
}StepperCtx_t;
typedef struct
{
	uint8_t ID;
	int16_t TarPos_mm;
}StepperControlData_t;
/* Functions */
HAL_StatusTypeDef StepperModuleInit(void);
int32_t GetStepperCurPos(uint8_t MotorID);
int32_t GetStepperCurVel(uint8_t MotorID);
int8_t 	GetStepperDir(uint8_t MotorID);
StepperMode_t GetStepperCurMode(uint8_t MotorID);
void SetStepperEnabled(uint8_t MotorID, uint8_t Enabled);
HAL_StatusTypeDef SetStepperTarPos(uint8_t MotorID, int32_t TarPos);
HAL_StatusTypeDef SetStepperTarPosWithVel(uint8_t MotorID, int32_t TarPos, int32_t TarVel);
HAL_StatusTypeDef SetStepperTarVel(uint8_t MotorID, int32_t TarVel);
void SetStepperZeroPos(uint8_t MotorID);

#endif

