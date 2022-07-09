#ifndef _MECH_
#define _MECH_
/* Includes */
#include "stdint.h"
#include "Stepper.h"
#include "Switch.h"

/* Defines */
#define HEAD 0
#define TAIL 1
#define SW1 0
#define SW2 1
#define SW3 2
#define SW4 3
#define SW5 4
#define SW6 5
#define WHEEL_HEAD_SW SW1
#define WHEEL_TAIL_SW SW2
#define ZAXIS_HEAD_SW SW3
#define ZAXIS_TAIL_SW SW4
#define RAXIS_SW SW5
#define THETA_SW SW6

#define M1 1
#define M2 2
#define M3 3
#define M4 4

#define WHEEL 0
#define ZAXIS 1
#define RAXIS 2
#define THETA 3


#define STOP		0
#define pi 		3.141592653
#define M42REVPULSE	6400
#define M57REVPULSE	6400
#define M42MAXVELRATIO 3
#define M42MAXVEL (M42REVPULSE * M42MAXVELRATIO)

#define NoPosTar	0x7FFF
/** Struct */
/** @brife 关节对象封装 */
typedef struct
{
	uint8_t MotorID;
	uint8_t SwHeadID;
	uint8_t SwTailID;
}Joint_t;

/* Functions */
void SW1Clicked(void);
void SW2Clicked(void);
void SW3Clicked(void);
void SW4Clicked(void);
void SW5Clicked(void);
void SW6Clicked(void);
void SW1Released(void);
void SW2Released(void);
void SW3Released(void);
void SW4Released(void);
void SW5Released(void);
void SW6Released(void);
void JointResetToHead(uint8_t jointName);
void JointResetToTail(uint8_t jointName);
void SwitchClickedProcess(uint8_t jointName, uint8_t SwName);
void SwitchReleasedProcess(uint8_t jointName, uint8_t SwName);
void DriveJointMove(uint8_t jointName, float tarPos, float tarVel);
#endif
