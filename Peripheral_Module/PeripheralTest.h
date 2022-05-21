#ifndef PRTEST_H
#define PRTEST_H
/* Includes */
#include "main.h"
#include "gpio.h"
#include "cmsis_os.h"
#include "freertos.h"
#include "KeyModule.h"
#include "LEDModule.h"

/* Defines */

/* Functions */
void SW1Clicked(void);
void SW2Clicked(void);
void SW3Clicked(void);
void SW4Clicked(void);
void SW1Released(void);
void SW2Released(void);
void SW3Released(void);
void SW4Released(void);
void StepperGoSwitchZeros(uint8_t MotorID, uint8_t SwID);
#endif
