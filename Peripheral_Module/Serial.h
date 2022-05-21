#ifndef SERIAL_H
#define SERIAL_H
#include "stdint.h"
#include "LEDModule.h"
#define SERIAL_DATABYTE 3
/* Structs ------------- */
typedef struct 
{
	uint8_t FrameStart;		// 帧起始frame start
	uint8_t DataByte;		// 帧长度（字节）
	uint8_t Data[SERIAL_DATABYTE];	// 数据域
	uint8_t FrameEnd;		// 帧尾（校验）
}SerialPackge_t;

/* Functions ----------- */
void SerialInit(void (*pFunc)(uint8_t *, uint32_t));
void SerialPrintf(const char *Format, ...);
void SerialPackTransmit(uint8_t* pData);
void SerialPackExtract(uint8_t *pPack, uint8_t *pData);
#endif
