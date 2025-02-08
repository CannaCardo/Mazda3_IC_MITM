/*
 * CC_CAN_Handler.h
 *
 *  Created on: Sep 1, 2024
 *      Author: CannaC
 */

#ifndef INC_CC_CAN_HANDLER_H_
#define INC_CC_CAN_HANDLER_H_


// ---------------------- Files

#include "stm32f4xx_hal.h"

// ---------------------- Type Definitions

typedef enum{
	CC_CAN_ERR__OK,
	CC_CAN_ERR__BUSY,
	CC_CAN_ERR__FATAL,
} CC_CAN_ERR;

typedef struct{
	uint32_t 	StdId;    /* standard identifier <0 - 0x7FF> (12bit)*/
	uint32_t 	ExtId;    /* extended identifier <0 - 0x1FFFFFFF> */
	uint32_t 	IDE;      /* type of identifie */
	uint32_t 	RTR;      /* type of frame */
	uint32_t 	DLC;      /* length of the frame */
	uint8_t	 	Data[8];

	uint8_t 	CAN_SRC;
} CAN_Packet_Type;

typedef struct{
	CAN_TxHeaderTypeDef 	Header;
	uint8_t					Data[8];
} CAN_TxPacketType;

typedef struct{
	CAN_RxHeaderTypeDef 	Header;
	uint8_t					Data[8];
} CAN_RxPacketType;

// ---------------------- Function Declarations

CC_CAN_ERR 			CC_CAN_Init(CAN_HandleTypeDef * can1_, CAN_HandleTypeDef * can2_);

CC_CAN_ERR			CC_CAN1_Send(CAN_TxPacketType * CPacket);
CC_CAN_ERR			CC_CAN2_Send(CAN_TxPacketType * CPacket);

#endif /* INC_CC_CAN_HANDLER_H_ */
