#ifndef _TEL_
#define _TEL_

#include "stdint.h"
#include "freertos.h"
#include "cmsis_os.h"
#include "Serial.h"
#include "LEDModule.h"
#include "KeyModule.h"
#include "AirPump.h"
#include "ULtraSonic.h"
#include "WeighingSensor.h"
#include "Mechanism.h"

#define WHEEL_JOINT	1
#define ZAXIS_JOINT	2
#define RAXIS_JOINT	3
#define THETA_JOINT	4
#define ALL_JOINT_RESET	0


void JointAllResetToHead(void);

#endif
