#ifndef ULTRASONIC_H
#define ULTRASONIC_H
/* Includes -------------------- */
#include "stdint.h"
#include "tim.h"

/* Defines --------------------- */

/* Struct& Enum ---------------- */
typedef enum
{
	TRIG_WAIT = 0,
	ECHO_RISING,
	ECHO_FALLING,
	OVER
}UltraSonicState_t;
typedef struct
{
	uint8_t id;
	GPIO_TypeDef *TrigPort;
	uint32_t TrigPin;
	TIM_TypeDef *Tim;
	TIM_HandleTypeDef *Htim;
	uint32_t Channel;
	UltraSonicState_t State;
	uint16_t Data[2];
	float Len;
}UltraSonic_t;
/* Functions ------------------- */
UltraSonic_t* Allocate_UltraSonic(GPIO_TypeDef *trigPort, uint32_t trigPin, TIM_TypeDef *tim, TIM_HandleTypeDef *htim, uint32_t channel);
void UltraSonic_Actuation(UltraSonic_t *This);


#endif
