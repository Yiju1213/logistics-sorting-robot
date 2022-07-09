#ifndef SERIAL_H
#define SERIAL_H
#include "stdint.h"
#define SERIAL_DATABYTE (2*7)
/* Structs ------------- */
typedef struct 
{
	uint8_t FrameStart;		// ֡��ʼframe start
	uint8_t DataByte;		// ֡���ȣ��ֽڣ�
	uint8_t Data[SERIAL_DATABYTE];	// ������
	uint8_t FrameEnd;		// ֡β��У�飩
}SerialPackge_t;
typedef enum
{
	GET_SYNC = 0x1,
	OPEN_CAM,
	CAM_OPENED,
	GET_MESSAGE,
	MESSAGE_GET
}UpperInstruction_t;
/* Functions ----------- */
void SerialInit(void (*pFunc)(uint8_t *, uint32_t));
void SerialPrintf(const char *Format, ...);
void SerialPackTransmit(void* pData);
void SerialRequireFromUpper(UpperInstruction_t instruction);
void SerialPackExtract(SerialPackge_t *pPack, void *pData);
#endif
