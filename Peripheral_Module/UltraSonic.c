/**
  ******************************************************************************
  * @file        : UltraSonic.c
  * @author      : WYR
  * @brief       : ����������߼�
  ******************************************************************************
  * @attention   ʹ�÷�����
					1. ָ��UltraSonicNum
					2. ��UltraSonic.hͷ�ļ��ж������������ģ��Ķ�ʱ��/���/ͨ��
					3. ���ⲿ���ó�����ģ�����뺯��Allocate_UltraSonic�����������Ϣ����ö���ָ��
					4. ѭ���е���UltraSonic_Actuation��ʵ�������Բ��
				ע�⣺CubeMx�ж�ʱ��Ҫ��ͨ���벶��ģʽ��Ƶ��Ҫ���ó�1MHz����������1us��
  ******************************************************************************
*/

/* Includes ------------------------ */
#include "UltraSonic.h"

/* Defines ------------------------- */
#define UltraSonicNum 1		// ������ģ������
/* Extern Members ------------------ */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);	// ��д__weak����
/* Private Members ----------------- */
static UltraSonic_t prvUltraSonic[UltraSonicNum];			// ����������ṹ��
static uint8_t prvUltraSonicIndex = 0;						// ����������������

/* Functions ----------------------- */
/** @brife ������ģ������ */
UltraSonic_t* Allocate_UltraSonic(GPIO_TypeDef *trigPort, uint32_t trigPin, TIM_TypeDef *tim, TIM_HandleTypeDef *htim, uint32_t channel)
{
	UltraSonic_t *pSpecific;
	uint8_t ID;
	ID = prvUltraSonicIndex;
	if(ID >= UltraSonicNum)	return NULL; // ���������������
	// ��ʼ�������
	pSpecific = &prvUltraSonic[prvUltraSonicIndex];
	prvUltraSonicIndex ++;
	// �����������
	pSpecific->TrigPort = trigPort;
	pSpecific->TrigPin	= trigPin;
	pSpecific->Tim		= tim;
	pSpecific->Htim		= htim;
	pSpecific->Channel	= channel;
	// �ش�����ָ��
	return pSpecific;
}

/** @brife ������ģ�������������𼰽�����㣩 */
void UltraSonic_Actuation(UltraSonic_t *This)
{
	// 1. �ж��Ƿ��ڿ���״̬
	if(This->State == TRIG_WAIT)
	{
		// 1.1 ��trig 1ms�ߵ�ƽ�ź�
		HAL_GPIO_WritePin(This->TrigPort,This->TrigPin,GPIO_PIN_SET);
		osDelay(1);
		HAL_GPIO_WritePin(This->TrigPort,This->TrigPin,GPIO_PIN_RESET);
		// 1.2 ���������ز���
		__HAL_TIM_SET_CAPTUREPOLARITY(This->Htim,This->Channel,TIM_INPUTCHANNELPOLARITY_RISING);
		// 1.3 �������벶��
		HAL_TIM_IC_Start_IT(This->Htim,This->Channel);
		// 1.4 ����״̬
		This->State = ECHO_RISING;
	}
	// 2. �ж��Ƿ��ڲ������״̬
	if(This->State == OVER)
	{
		// 2.1 ��������
		// ����ߵ�ƽʱ���ֵ����ʱ��ά�ֵĳ��ȣ���Ϊ��ʱ������1us�Լ�һ�Σ�����ʱ����λΪus
		// ����0.017������Ϊ�����ٶ�Ϊ340m/s����1s=1000000us���Ҽ���һ�ξ�����һ������
		// Ҫ����2���˴���λΪcm
		This->Len = (float)(This->Data[1] - This->Data[0]) * 0.017;
		// 2.2 ˢ��״̬��ʹ���Խ�����һ�δ�����ƽ
		HAL_TIM_IC_Stop_IT(This->Htim,This->Channel);
		This->State = TRIG_WAIT;
	}
}

/** @brife ������ģ���������жϲ��� */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	uint8_t i;
	UltraSonic_t *This;
	// �ҵ����������ģ���о�����벶���жϵ�
	for(i=0;i<UltraSonicNum;i++)
	{
		This = &prvUltraSonic[i];
		if(This->Tim == htim->Instance)
		{
			// 1. �����ز���״̬
			if(This->State == ECHO_RISING)
			{
				// 1.1 ����CNT
				__HAL_TIM_SetCounter(This->Htim, 0);
				// 1.2 ��ȡ��ǰֵ
				This->Data[0] = __HAL_TIM_GetCounter(This->Htim);
				// 1.3 ������һ������Ϊ�½���
				__HAL_TIM_SET_CAPTUREPOLARITY(This->Htim,This->Channel,TIM_INPUTCHANNELPOLARITY_FALLING);
				// 1.4 ����״̬
				This->State = ECHO_FALLING;
			}
			// 2. �½��ز���״̬
			else if(This->State == ECHO_FALLING)
			{
				// 2.1 ��ȡ��ǰֵ
				This->Data[1] = __HAL_TIM_GetCounter(This->Htim);
				// 2.2 ����״̬
				This->State = OVER;
			}
		}
		//break;
	}
}
