#ifndef SERIAL_H
#define SERIAL_H
#include "stdint.h"
#define SERIAL_DATABYTE (2*7)
/* Structs ------------- */
typedef struct 
{
	uint8_t FrameStart;		// 帧起始frame start
	uint8_t DataByte;		// 帧长度（字节）
	uint8_t Data[SERIAL_DATABYTE];	// 数据域
	uint8_t FrameEnd;		// 帧尾（校验）
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
