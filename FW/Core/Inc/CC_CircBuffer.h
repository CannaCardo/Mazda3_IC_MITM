/*
 * CC_CircBuffer.h
 *
 *  Created on: May 23, 2024
 *      Author: CannaC
 */

#ifndef INC_CC_CIRCBUFFER_H_
#define INC_CC_CIRCBUFFER_H_

/*
 * This is a crude circular buffer implementation
 * Reused between projects cuz lazy
 * */

// ---------------------- Files

#include <stdint.h>

// ---------------------- Type Definitions

typedef enum{
	CC_CIRCBUFFER_ERR__OK,
	CC_CIRCBUFFER_ERR__FULL,
	CC_CIRCBUFFER_ERR__EMPTY,
	CC_CIRCBUFFER_ERR__OUTOFRANGE,
} CC_CIRCBUFFER_ERR;

typedef struct{
	void *				Buf;
	volatile uint32_t 	Buf_Head;
	volatile uint32_t 	Buf_Tail;
	volatile uint32_t 	Buf_Len;
	uint32_t 			Buf_MaxSize;
	uint8_t				ElementSize;
} CC_CIRCBUFFER_TYPE;

// ---------------------- Function Declarations


CC_CIRCBUFFER_TYPE * 	CC_CircBuffer_Init(uint32_t MaxSize, uint8_t ElementSize);

CC_CIRCBUFFER_ERR		CC_CircBuffer_Push(CC_CIRCBUFFER_TYPE * CBuf, const void * Val);
CC_CIRCBUFFER_ERR		CC_CircBuffer_Pop(CC_CIRCBUFFER_TYPE * CBuf, void * Val); // Removes from the head (FIFO)
CC_CIRCBUFFER_ERR		CC_CircBuffer_Set(CC_CIRCBUFFER_TYPE * CBuf, uint32_t Index, const void * Val); // Replaces value at index
CC_CIRCBUFFER_ERR		CC_CircBuffer_Get(CC_CIRCBUFFER_TYPE * CBuf, uint32_t Index, void * Val);
const void *			CC_CircBuffer_GetPtr(CC_CIRCBUFFER_TYPE * CBuf, uint32_t Index);
CC_CIRCBUFFER_ERR		CC_CircBuffer_Insert(CC_CIRCBUFFER_TYPE * CBuf, uint32_t Index, uint32_t Len, const void * Val); // Unimplemented
CC_CIRCBUFFER_ERR		CC_CircBuffer_Remove(CC_CIRCBUFFER_TYPE * CBuf, uint32_t IndexStart, uint32_t Len);

CC_CIRCBUFFER_ERR		CC_CircBuffer_PushStr(CC_CIRCBUFFER_TYPE * CBuf, const void * Str); // Null terminated
CC_CIRCBUFFER_ERR		CC_CircBuffer_PushStrN(CC_CIRCBUFFER_TYPE * CBuf, const void * Str, uint32_t Len);
//TODO popstr?

CC_CIRCBUFFER_ERR		CC_CircBuffer_Clear(CC_CIRCBUFFER_TYPE * CBuf);
CC_CIRCBUFFER_ERR		CC_CircBuffer_Enlarge(CC_CIRCBUFFER_TYPE * CBuf, uint32_t Len); // Unimplemented


#endif /* INC_CC_CIRCBUFFER_H_ */
