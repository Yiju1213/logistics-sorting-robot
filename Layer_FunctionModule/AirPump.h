#ifndef _PUMP_
#define _PUMP_

#include "stdint.h"
#include "main.h"

#define PUMP_ON		(HAL_GPIO_WritePin(PUMP_GPIO_Port,PUMP_Pin,GPIO_PIN_SET))
#define PUMP_OFF		(HAL_GPIO_WritePin(PUMP_GPIO_Port,PUMP_Pin,GPIO_PIN_RESET))
#define PUMP_Release_On	(HAL_GPIO_WritePin(PUMP_Release_GPIO_Port,PUMP_Release_Pin,GPIO_PIN_SET))
#define PUMP_Release_Off	(HAL_GPIO_WritePin(PUMP_Release_GPIO_Port,PUMP_Release_Pin,GPIO_PIN_RESET))

#endif
