/**
  ******************************************************************************
  * @file        : WeighingSensor.c
  * @author      : DB & WYR
  * @brief       : ѹ������ģ���߼�
  ******************************************************************************
  * @attention   ʹ�÷�����
					1. ָ��prvWeightIndex
					3. ���ⲿ���ò���ģ�����뺯��WeighingSensor_Allocate�����������Ϣ����ö���ָ��
					4. ѭ���е���WeighingSensor_Actuation��ʵ�������Բ��
  ******************************************************************************
*/
#include "WeighingSensor.h"

#define DATASCALE 		106.5
#define DATABIT			24
#define WEIGHTSENSOR_NUM 1

static WeightSensor_t prvWeightSensor[WEIGHTSENSOR_NUM];
static uint8_t prvWeightIndex = 0;

/* Functions */
/** @brife ѹ������ģ��������� */
WeightSensor_t* WeighingSensor_Allocate(GPIO_TypeDef* sck_port, uint32_t sck_pin,
									GPIO_TypeDef* dout_port, uint32_t dout_pin)
{
	uint8_t id;
	WeightSensor_t* pSpecific;
	// ����������б�
	id = prvWeightIndex;
	if(id >= WEIGHTSENSOR_NUM) return NULL;
	// ��ʼ�������
	pSpecific = &prvWeightSensor[id];
	prvWeightIndex ++;
	// �����������
	pSpecific->SCK_Port	= sck_port;
	pSpecific->SCK_Pin	= sck_pin;
	pSpecific->Dout_Port	= dout_port;
	pSpecific->Dout_Pin	= dout_pin;
	// ����ʵ������ָ��
	return pSpecific;
}
/** @brife ѹ������ģ�������������𼰽�����㣩 */
void WeighingSensor_Actuation(WeightSensor_t* This)
{
	uint8_t cnt = 0;
	int32_t data = 0;
	// ��HX711Dout��������ߵ�ƽ����δ׼���������ʱ��SCK���ֵ͵�ƽ
	HAL_GPIO_WritePin(This->Dout_Port,This->Dout_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(This->SCK_Port,This->SCK_Pin,GPIO_PIN_RESET);
	delay_us(1);
	// �ȴ�Dout��������͵�ƽ����׼����ʼ��������
	while(HAL_GPIO_ReadPin(This->Dout_Port, This->Dout_Pin) != GPIO_PIN_RESET);
	// Dout��������͵�ƽʱ����������SCK����ʱ���ؿ�ʼ����
	for(cnt=0; cnt<DATABIT ;cnt++)
	{
		// SCKд��ߵ�ƽ1us
		HAL_GPIO_WritePin(This->SCK_Port,This->SCK_Pin,GPIO_PIN_SET);
		delay_us(1);
		data = data << 1;
		// SCKд��͵�ƽ��ͬʱ����1bit Dout���ݣ����֤�͵�ƽ����ʱ��1us��
		HAL_GPIO_WritePin(This->SCK_Port,This->SCK_Pin,GPIO_PIN_RESET);
		if(HAL_GPIO_ReadPin(This->Dout_Port, This->Dout_Pin) == GPIO_PIN_SET)
		{
			data++;
		}
			
		// �ȴ���һbit Dout����
		
		delay_us(1);
	}
	// �����������꣬��ͨ�����壨128����Aͨ����1�����壩
	HAL_GPIO_WritePin(This->SCK_Port,This->SCK_Pin,GPIO_PIN_SET);
	delay_us(1);
	HAL_GPIO_WritePin(This->SCK_Port,This->SCK_Pin,GPIO_PIN_RESET);
	// �߼�����8λ�Ҷ�ȫ��0���߼�����8λ��˰��շ���λ��λ������з���24bitAD���з���32bitת��
	data = ((data << 8) >> 8);
	// ���ݸ���
	This->Weight = data;
}

