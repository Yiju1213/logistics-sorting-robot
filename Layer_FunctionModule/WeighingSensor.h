#ifndef _WEIGHT_H_
#define _WEIGHT_H_

#include "stdint.h"
#include "tim.h"

/* Struct */
typedef struct 
{
	GPIO_TypeDef*	SCK_Port;
	uint32_t			SCK_Pin;
	GPIO_TypeDef*	Dout_Port;
	uint32_t			Dout_Pin;
	int32_t 		Weight;
}WeightSensor_t;
/* Functions */
WeightSensor_t* WeighingSensor_Allocate(GPIO_TypeDef* sck_port, uint32_t sck_pin,
									GPIO_TypeDef* dout_port, uint32_t dout_pin);
void WeighingSensor_Actuation(WeightSensor_t* This);
uint32_t HX711_DataRead(void);
#endif
