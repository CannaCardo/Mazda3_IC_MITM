/*
 * CC_CircBuffer.c
 *
 *  Created on: May 23, 2024
 *      Author: k.belewicz
 */



// ---------------------- Files

#include "CC_CircBuffer.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// ---------------------- Local Variables

#ifndef NULL
#define NULL	(0)
#endif

// ---------------------- Helper Function Declarations

#define		VOID_PTR_INC(ptr, offset)	( (void*)( (uint8_t*)(ptr) + offset ) )

static inline uint32_t CC_CircBuffer_WrapIndex(uint32_t index, uint32_t max){
	while ( index >= max )	index -= max;
	return index;
}

static inline void CC_CircBuffer_Index_Incr(volatile uint32_t * index, uint32_t * max){
	*index = CC_CircBuffer_WrapIndex( (*index)+1, *max );
}
static inline void CC_CircBuffer_Index_Decr(volatile uint32_t * index, uint32_t * max){
	if (*index == 0)	*index = (*max)-1;
	else				*index -= 1;
}

// ---------------------- Function Definitions


CC_CIRCBUFFER_TYPE *	CC_CircBuffer_Init(uint32_t MaxSize, uint8_t ElementSize){
	CC_CIRCBUFFER_TYPE * CBuf;

	// Allocate memory
	CBuf = malloc(sizeof(CC_CIRCBUFFER_TYPE) * 1);
	CBuf->Buf = malloc(ElementSize * MaxSize);
	CBuf->Buf_Head = 0;
	CBuf->Buf_Tail = 0;
	CBuf->Buf_Len = 0;
	CBuf->Buf_MaxSize = MaxSize;
	CBuf->ElementSize = ElementSize;

	return CBuf;
}

CC_CIRCBUFFER_ERR	CC_CircBuffer_Push(CC_CIRCBUFFER_TYPE * CBuf, const void * Val){
	CC_CIRCBUFFER_ERR ret = CC_CIRCBUFFER_ERR__OK;
	if (CBuf->Buf_Len >= CBuf->Buf_MaxSize) 	return CC_CIRCBUFFER_ERR__FULL;

	if (CBuf->Buf_Len == 0){
		// Tail and head in the same place
		memcpy( VOID_PTR_INC(CBuf->Buf, ((CBuf->Buf_Tail) * (CBuf->ElementSize)) ), Val, CBuf->ElementSize );
	}else{
		CC_CircBuffer_Index_Incr(&(CBuf->Buf_Tail), &(CBuf->Buf_MaxSize));
		memcpy( VOID_PTR_INC(CBuf->Buf, ((CBuf->Buf_Tail) * (CBuf->ElementSize)) ), Val, CBuf->ElementSize );
	}
	CBuf->Buf_Len += 1;

	return ret;
}

CC_CIRCBUFFER_ERR	CC_CircBuffer_Pop(CC_CIRCBUFFER_TYPE * CBuf, void * Val){
	CC_CIRCBUFFER_ERR ret = CC_CIRCBUFFER_ERR__OK;
	if (CBuf->Buf_Len == 0) 	return CC_CIRCBUFFER_ERR__EMPTY;

	memcpy( Val, VOID_PTR_INC(CBuf->Buf, ((CBuf->Buf_Head) * (CBuf->ElementSize)) ), CBuf->ElementSize );
	(CBuf->Buf_Len) -= 1;
	if (CBuf->Buf_Len == 0){
		// Tail and head in the same place, do nothing
	}else{
		CC_CircBuffer_Index_Incr(&(CBuf->Buf_Head), &(CBuf->Buf_MaxSize));
	}

	return ret;
}

CC_CIRCBUFFER_ERR	CC_CircBuffer_Set(CC_CIRCBUFFER_TYPE * CBuf, uint32_t Index, const void * Val){
	CC_CIRCBUFFER_ERR ret = CC_CIRCBUFFER_ERR__OK;
	if (Index >= CBuf->Buf_Len) 	return CC_CIRCBUFFER_ERR__OUTOFRANGE;
	memcpy( VOID_PTR_INC(CBuf->Buf, (CC_CircBuffer_WrapIndex((CBuf->Buf_Head)+Index,CBuf->Buf_MaxSize) * (CBuf->ElementSize)) ), Val, CBuf->ElementSize );
	return ret;
}

CC_CIRCBUFFER_ERR 	CC_CircBuffer_Get(CC_CIRCBUFFER_TYPE * CBuf, uint32_t Index, void * Val){
	CC_CIRCBUFFER_ERR ret = CC_CIRCBUFFER_ERR__OK;
	if (Index >= CBuf->Buf_Len) 	return CC_CIRCBUFFER_ERR__OUTOFRANGE;
	memcpy( Val, VOID_PTR_INC(CBuf->Buf, (CC_CircBuffer_WrapIndex((CBuf->Buf_Head)+Index,CBuf->Buf_MaxSize) * (CBuf->ElementSize)) ), CBuf->ElementSize );
	return ret;
}

const void * 		CC_CircBuffer_GetPtr(CC_CIRCBUFFER_TYPE * CBuf, uint32_t Index){
	//CC_CIRCBUFFER_ERR ret = CC_CIRCBUFFER_ERR__OK;
	if (Index >= CBuf->Buf_Len) 	return (void*)0;//CC_CIRCBUFFER_ERR__OUTOFRANGE;
	return ( VOID_PTR_INC(CBuf->Buf, (CC_CircBuffer_WrapIndex((CBuf->Buf_Head)+Index,CBuf->Buf_MaxSize) * (CBuf->ElementSize)) ) );
}

CC_CIRCBUFFER_ERR	CC_CircBuffer_Insert(CC_CIRCBUFFER_TYPE * CBuf, uint32_t Index, uint32_t Len, const void * Val){
	//CC_CIRCBUFFER_ERR ret = CC_CIRCBUFFER_ERR__OK;
	//TODO INSERT
	return CC_CIRCBUFFER_ERR__OUTOFRANGE;
}

CC_CIRCBUFFER_ERR	CC_CircBuffer_Remove(CC_CIRCBUFFER_TYPE * CBuf, uint32_t IndexStart, uint32_t Len){
	CC_CIRCBUFFER_ERR ret = CC_CIRCBUFFER_ERR__OK;
	if (CBuf->Buf_Len < (IndexStart+Len)) 	return CC_CIRCBUFFER_ERR__EMPTY;

	CBuf->Buf_Len -= Len;
	// If from the end - move tail
	// If from the start - move head
	// If from the middle - move head forward
	if (IndexStart == 0){
		while (Len > 1){
			CC_CircBuffer_Index_Incr(&(CBuf->Buf_Head), &(CBuf->Buf_MaxSize));
			--Len;
		}
		if (Len == 1) --Len;
	}else if (IndexStart+Len == CBuf->Buf_Len){
		while (Len > 1){
			CC_CircBuffer_Index_Decr(&(CBuf->Buf_Tail), &(CBuf->Buf_MaxSize));
			--Len;
		}
		if (Len == 1) --Len;
	}else{
		uint32_t Count = IndexStart;
		while (Count > 0){
			--Count;
			memcpy( VOID_PTR_INC(CBuf->Buf, CC_CircBuffer_WrapIndex((CBuf->Buf_Head)+Count+Len,CBuf->Buf_MaxSize) ),
					VOID_PTR_INC(CBuf->Buf, CC_CircBuffer_WrapIndex((CBuf->Buf_Head)+Count,CBuf->Buf_MaxSize) ), CBuf->ElementSize );
		}
		Count = IndexStart;
		while (Count > 0){
			--Count;
			CC_CircBuffer_Index_Incr(&(CBuf->Buf_Head), &(CBuf->Buf_MaxSize));
		}
	}
	return ret;
}

CC_CIRCBUFFER_ERR	CC_CircBuffer_PushStr(CC_CIRCBUFFER_TYPE * CBuf, const void * Str){
	CC_CIRCBUFFER_ERR ret = CC_CIRCBUFFER_ERR__OK;
	uint32_t Len = 0;
	uint32_t k, j;
	uint32_t offset = 0;
	while( 1 ){
		j=1;
		for(k=0; k < CBuf->ElementSize; ++k){
			if ( *( (uint8_t *)(Str) + offset ) != '\0' ){
				j=0;
				offset += CBuf->ElementSize - k;
				break;
			}
			++offset;
		}
		if( j == 1 ) break;
		++Len;
	}
	if (Len >= (CBuf->Buf_MaxSize - CBuf->Buf_Len)) return CC_CIRCBUFFER_ERR__FULL;

	for (uint32_t Count = 0; Count<Len; ++Count){
		if (CBuf->Buf_Len == 0){
			// Tail and head in the same place
			memcpy( VOID_PTR_INC(CBuf->Buf, CBuf->Buf_Tail), VOID_PTR_INC(Str, Count), CBuf->ElementSize );
		}else{
			CC_CircBuffer_Index_Incr(&(CBuf->Buf_Tail), &(CBuf->Buf_MaxSize));
			memcpy( VOID_PTR_INC(CBuf->Buf, CBuf->Buf_Tail), VOID_PTR_INC(Str, Count), CBuf->ElementSize );
		}
		CBuf->Buf_Len += 1;
	}

	return ret;
}

CC_CIRCBUFFER_ERR	CC_CircBuffer_PushStrN(CC_CIRCBUFFER_TYPE * CBuf, const void * Str, uint32_t Len){
	CC_CIRCBUFFER_ERR ret = CC_CIRCBUFFER_ERR__OK;

	if (Len >= (CBuf->Buf_MaxSize - CBuf->Buf_Len)) return CC_CIRCBUFFER_ERR__FULL;

	for (uint32_t Count = 0; Count<Len; ++Count){
		if (CBuf->Buf_Len == 0){
			// Tail and head in the same place
			memcpy( VOID_PTR_INC(CBuf->Buf, CBuf->Buf_Tail), VOID_PTR_INC(Str, Count), CBuf->ElementSize );
		}else{
			CC_CircBuffer_Index_Incr(&(CBuf->Buf_Tail), &(CBuf->Buf_MaxSize));
			memcpy( VOID_PTR_INC(CBuf->Buf, CBuf->Buf_Tail), VOID_PTR_INC(Str, Count), CBuf->ElementSize );
		}
		CBuf->Buf_Len += 1;
	}

	return ret;
}


CC_CIRCBUFFER_ERR	CC_CircBuffer_Clear(CC_CIRCBUFFER_TYPE * CBuf){
	CC_CIRCBUFFER_ERR ret = CC_CIRCBUFFER_ERR__OK;
	CBuf->Buf_Head = 0;
	CBuf->Buf_Tail = 0;
	CBuf->Buf_Len = 0;
	return ret;
}

CC_CIRCBUFFER_ERR	CC_CircBuffer_Enlarge(CC_CIRCBUFFER_TYPE * CBuf, uint32_t Len){
	CC_CIRCBUFFER_ERR ret = CC_CIRCBUFFER_ERR__OK;

	//TODO

	return CC_CIRCBUFFER_ERR__OUTOFRANGE;
}

