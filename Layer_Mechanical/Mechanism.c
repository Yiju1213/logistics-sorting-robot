/**
  ******************************************************************************
  * @file        : Mechanism.c
  * @author      : LCZ & WYR
  * @brief       : ʵ�ֻ��������װ
  ******************************************************************************
  * @attention   ʹ�÷�����
  
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
/** @brife �ؽ�λ��������λ����ת�� */
/* ���漰�������ͺ� */
/** @Tips ���ڹؽ�λ����mm OR rad�����ص���Ȧ��r */
/**		  ���ڹؽ��ٶ���mm/s OR rad/s�����ص���ÿ��Ȧ��r/s */
float Target_MMtransform(uint8_t jointName, float mechTar)
{
	float shaftTar = 0;
	switch(jointName)
	{
		// _SyncBelt ͬ����
		case WHEEL:		
			shaftTar = (mechTar / REV_DISP_MWHEEL);
			break;
		// _ScrewRod ˿��
		case ZAXIS:		
			shaftTar = -(mechTar / REV_DISP_MZAXIS);
			break;
		// _PinionRackBelt ���ֳ���������ͬ����
		case RAXIS:		
			shaftTar = (mechTar / REV_DISP_RAXIS);
			break;
		// _SpinAngle ת���Ƕ�
		case THETA:		
			shaftTar = (mechTar / REV_DISP_THETA);
			break;
		default:		shaftTar = 0;
	}
	return shaftTar;
}

/** @brife �ؽ��˶����� */
/** @param joint:	{WHEEL, ZAXIS, RAXIS, THETA}
	@param tarVel:	mm/s OR rad/s 
	@param tarPos:	mm OR rad 		*/
void DriveJointMove(uint8_t jointName, float tarPos, float tarVel)
{
	// 1. ���ؽ��˶���תΪ����˶���
	float shaftRound	= 0;
	float shaftRPS	= 0;
	shaftRound	= Target_MMtransform(jointName, tarPos);
	shaftRPS	= Target_MMtransform(jointName, tarPos);
	// 2. �����λ����תΪ���Ŀ��������
	float stepperPulse	= 0;
	float stepperPPS		= 0;
	// MWHEEL��57����
	if(jointName == WHEEL)
	{
		stepperPulse	= shaftRound * M57REVPULSE;
		stepperPPS	= shaftRPS * M57REVPULSE;
	}
	// MZAXIS, MRAXIS, MTHETA��42����
	else
	{
		stepperPulse	= shaftRound * M42REVPULSE;
		stepperPPS	= shaftRPS * M42REVPULSE;
	}
	// 3. Stepper����
	if(tarPos == NoPosTar)
		SetStepperTarVel(Joint[jointName].MotorID, stepperPPS);
	else
		SetStepperTarPosWithVel(Joint[jointName].MotorID, stepperPulse, stepperPPS);
}

/** @brife �ؽڸ�λ */
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

/** @brife ��λ���ذ��¶������� */
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
/** @brife ��λ�����ͷŶ������� */
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
/** @Tips ����г̿�����ؽ���λ�İ󶨹��� */
void SW1Clicked(void)
{
	// SW1����WHEEL_HEAD_SW
	SwitchClickedProcess(WHEEL, WHEEL_HEAD_SW);
	
}
void SW2Clicked(void)
{
	// SW2����WHEEL_TAIL_SW
	SwitchClickedProcess(WHEEL, WHEEL_TAIL_SW);
	
}
void SW3Clicked(void)
{
	// SW3����ZAXIS_HEAD_SW
	SwitchClickedProcess(ZAXIS, ZAXIS_HEAD_SW);
	
}
void SW4Clicked(void)
{
	// SW4����ZAXIS_TAIL_SW
	SwitchClickedProcess(ZAXIS, ZAXIS_TAIL_SW);
	
}
void SW5Clicked(void)
{
	// SW5����RAXIS_SW
	SwitchClickedProcess(RAXIS, RAXIS_SW);
}
void SW6Clicked(void)
{
	// SW6����THETA_SW
	SwitchClickedProcess(THETA, THETA_SW);
}
void SW1Released(void)
{
	// SW1����WHEEL_HEAD_SW
	SwitchReleasedProcess(WHEEL, WHEEL_HEAD_SW);
	
}
void SW2Released(void)
{
	// SW2����WHEEL_TAIL_SW
	SwitchReleasedProcess(WHEEL, WHEEL_TAIL_SW);
	
}
void SW3Released(void)
{
	// SW3��ΪZAXIS_HEAD_SW
	SwitchReleasedProcess(ZAXIS, ZAXIS_HEAD_SW);
	
}
void SW4Released(void)
{
	// SW4����ZAXIS_TAIL_SW
	SwitchReleasedProcess(ZAXIS, ZAXIS_TAIL_SW);
	
}
void SW5Released(void)
{
	// SW5����RAXIS_SW
	SwitchReleasedProcess(RAXIS, RAXIS_SW);
}
void SW6Released(void)
{
	// SW6����THETA_SW
	SwitchReleasedProcess(THETA, THETA_SW);
}

