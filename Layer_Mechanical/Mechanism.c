/**
  ******************************************************************************
  * @file        : Mechanism.c
  * @author      : LCZ & WYR
  * @brief       : 实现机构对象封装
  ******************************************************************************
  * @attention   使用方法：
  
  ******************************************************************************
*/
/* Includes */
#include "Mechanism.h"


/* Defines */
#define ABS(x)	((x) > 0 ? (1) : (-1))
#define REV_DISP_MWHEEL 75.f
#define REV_DISP_MZAXIS 4.f
#define REV_DISP_RAXIS	(125.66*1.f)
#define REV_DISP_THETA	(pi*19/180)
                    
#define MTHETARATIO		19.f

/* Members */
Switch_t Switch[4];
Joint_t Joint[4] = {	{M1, WHEEL_HEAD_SW, WHEEL_TAIL_SW},
					{M2, ZAXIS_HEAD_SW, ZAXIS_TAIL_SW},
					{M3, RAXIS_SW, RAXIS_SW},
					{M4, THETA_SW, THETA_SW}	   };
/* Functions */
/** @brife 关节位移量与电机位移量转换 */
/* 不涉及具体电机型号 */
/** @Tips 对于关节位移量mm OR rad，返回的是圈数r */
/**		  对于关节速度量mm/s OR rad/s，返回的是每秒圈数r/s */
float Target_MMtransform(uint8_t jointName, float mechTar)
{
	float shaftTar = 0;
	switch(jointName)
	{
		// _SyncBelt 同步带
		case WHEEL:		
			shaftTar = (mechTar / REV_DISP_MWHEEL);
			break;
		// _ScrewRod 丝杆
		case ZAXIS:		
			shaftTar = -(mechTar / REV_DISP_MZAXIS);
			break;
		// _PinionRackBelt 齿轮齿条副驱动同步带
		case RAXIS:		
			shaftTar = (mechTar / REV_DISP_RAXIS);
			break;
		// _SpinAngle 转动角度
		case THETA:		
			shaftTar = (mechTar / REV_DISP_THETA);
			break;
		default:		shaftTar = 0;
	}
	return shaftTar;
}

/** @brife 关节运动控制 */
/** @param joint:	{WHEEL, ZAXIS, RAXIS, THETA}
	@param tarVel:	mm/s OR rad/s 
	@param tarPos:	mm OR rad 		*/
void DriveJointMove(uint8_t jointName, float tarPos, float tarVel)
{
	// 1. 将关节运动量转为电机运动量
	float shaftRound	= 0;
	float shaftRPS	= 0;
	shaftRound	= Target_MMtransform(jointName, tarPos);
	shaftRPS	= Target_MMtransform(jointName, tarPos);
	// 2. 将电机位移量转为电机目标脉冲数
	float stepperPulse	= 0;
	float stepperPPS		= 0;
	// MWHEEL是57步进
	if(jointName == WHEEL)
	{
		stepperPulse	= shaftRound * M57REVPULSE;
		stepperPPS	= shaftRPS * M57REVPULSE;
	}
	// MZAXIS, MRAXIS, MTHETA是42步进
	else
	{
		stepperPulse	= shaftRound * M42REVPULSE;
		stepperPPS	= shaftRPS * M42REVPULSE;
	}
	// 3. Stepper驱动
	if(tarPos == NoPosTar)
		SetStepperTarVel(Joint[jointName].MotorID, stepperPPS);
	else
		SetStepperTarPosWithVel(Joint[jointName].MotorID, stepperPulse, stepperPPS);
}

/** @brife 关节复位 */
void JointResetToHead(uint8_t jointName)
{
	int8_t tarDir = -1;
	uint8_t motor = Joint[jointName].MotorID;
	uint8_t swID = Joint[jointName].SwHeadID;
	Switch[swID].mode = ZerosP1;
	SetStepperTarVel(motor, tarDir * M42REVPULSE * M42MAXVELRATIO);
}
void JointResetToTail(uint8_t jointName)
{
	int8_t tarDir = 1;
	uint8_t motor = Joint[jointName].MotorID;
	uint8_t swID = Joint[jointName].SwTailID;
	Switch[swID].mode = ZerosP1;
	SetStepperTarVel(motor, tarDir * M42REVPULSE * M42MAXVELRATIO);
}

/** @brife 限位开关按下动作流程 */
void SwitchClickedProcess(uint8_t jointName, uint8_t SwID)
{
	uint8_t motor = Joint[jointName].MotorID;
	switch(Switch[SwID].mode)
	{
		case Limit:
			SetStepperTarVel(motor, STOP);
			break;
		case ZerosP1:
			SetStepperTarVel(motor, -ABS(GetStepperDir(motor))*1600);
			break;
		case ZerosP2:
			SetStepperZeroPos(motor);
			break;
	}
}
/** @brife 限位开关释放动作流程 */
void SwitchReleasedProcess(uint8_t jointName, uint8_t SwID)
{
	uint8_t motor = Joint[jointName].MotorID;
	switch(Switch[SwID].mode)
	{
		case ZerosP1:
			SetStepperTarVel(motor, -ABS(GetStepperDir(motor))*1600);
			Switch[SwID].mode = ZerosP2;
			break;
		default:
			Switch[SwID].mode = Limit;
	}
}
/** @Tips 添加行程开关与关节限位的绑定工作 */
void SW1Clicked(void)
{
	// SW1接入WHEEL_HEAD_SW
	SwitchClickedProcess(WHEEL, WHEEL_HEAD_SW);
	
}
void SW2Clicked(void)
{
	// SW2接入WHEEL_TAIL_SW
	SwitchClickedProcess(WHEEL, WHEEL_TAIL_SW);
	
}
void SW3Clicked(void)
{
	// SW3接入ZAXIS_HEAD_SW
	SwitchClickedProcess(ZAXIS, ZAXIS_HEAD_SW);
	
}
void SW4Clicked(void)
{
	// SW4接入ZAXIS_TAIL_SW
	SwitchClickedProcess(ZAXIS, ZAXIS_TAIL_SW);
	
}
void SW5Clicked(void)
{
	// SW5接入RAXIS_SW
	SwitchClickedProcess(RAXIS, RAXIS_SW);
}
void SW6Clicked(void)
{
	// SW6接入THETA_SW
	SwitchClickedProcess(THETA, THETA_SW);
}
void SW1Released(void)
{
	// SW1接入WHEEL_HEAD_SW
	SwitchReleasedProcess(WHEEL, WHEEL_HEAD_SW);
	
}
void SW2Released(void)
{
	// SW2接入WHEEL_TAIL_SW
	SwitchReleasedProcess(WHEEL, WHEEL_TAIL_SW);
	
}
void SW3Released(void)
{
	// SW3作为ZAXIS_HEAD_SW
	SwitchReleasedProcess(ZAXIS, ZAXIS_HEAD_SW);
	
}
void SW4Released(void)
{
	// SW4接入ZAXIS_TAIL_SW
	SwitchReleasedProcess(ZAXIS, ZAXIS_TAIL_SW);
	
}
void SW5Released(void)
{
	// SW5接入RAXIS_SW
	SwitchReleasedProcess(RAXIS, RAXIS_SW);
}
void SW6Released(void)
{
	// SW6接入THETA_SW
	SwitchReleasedProcess(THETA, THETA_SW);
}

