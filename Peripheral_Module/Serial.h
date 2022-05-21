#ifndef SERIAL_H
#define SERIAL_H
#include "stdint.h"
#include "LEDModule.h"
#define SERIAL_DATABYTE 3
/* Structs ------------- */
typedef struct 
{
	uint8_t FrameStart;		// ֡��ʼframe start
	uint8_t DataByte;		// ֡���ȣ��ֽڣ�
	uint8_t Data[SERIAL_DATABYTE];	// ������
	uint8_t FrameEnd;		// ֡β��У�飩
}SerialPackge_t;

/* Functions ----------- */
void SerialInit(void (*pFunc)(uint8_t *, uint32_t));
void SerialPrintf(const char *Format, ...);
void SerialPackTransmit(uint8_t* pData);
void SerialPackExtract(uint8_t *pPack, uint8_t *pData);
#endif
