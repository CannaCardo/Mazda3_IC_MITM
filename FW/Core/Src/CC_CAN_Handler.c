/*
 * CC_CAN_Handler.c
 *
 *  Created on: Sep 1, 2024
 *      Author: CannaC
 */


// ---------------------- Files

#include "CC_LED_Handler.h"
#include "CC_CAN_Handler.h"
#include "CC_CAN_Handler_priv.h"
#include "CC_UART_Handler.h"
#include "stm32f4xx_hal.h"

#include <stdlib.h>

// ---------------------- Local Variables

CAN_HandleTypeDef * 	CC_CAN_hcan1;
CAN_HandleTypeDef * 	CC_CAN_hcan2;

uint32_t 				CC_CAN1_TxMailbox;
uint32_t 				CC_CAN2_TxMailbox;

CAN_RxPacketType 		CAN1_RxPacket;
CAN_RxPacketType 		CAN2_RxPacket;
CAN_TxPacketType 		CAN1_TxPacket;
CAN_TxPacketType 		CAN2_TxPacket;
uint8_t			 		CAN1_TxPacket_CopyCnt; // used when copying DATA from CAN2_RxPacket to CAN1_TxPacket
uint8_t			 		CAN2_TxPacket_CopyCnt; // used when copying DATA from CAN1_RxPacket to CAN2_TxPacket

// Last seen RPM value
volatile uint8_t		RPM_Captured;
volatile uint8_t		RPM_LastSeen_H;
volatile uint8_t		RPM_LastSeen_L;
volatile uint8_t		RPM_BN_Captured;
CAN_TxPacketType 		CAN2_TxPacket_BNRepeat;

// ---------------------- Helper Function Declarations

static inline void	CC_CAN_InjectRPM(volatile uint8_t * Data, volatile uint8_t * RPM_H, volatile uint8_t * RPM_L);
static inline void	CC_CAN_ExtractRPM(volatile uint8_t * Data, volatile uint8_t * RPM_H, volatile uint8_t * RPM_L);
static inline void	CC_CAN_InjectRPM_BNStyle(volatile uint8_t * Data, volatile uint8_t * RPM_H, volatile uint8_t * RPM_L);
//TODO INFO BUTTONS

// ---------------------- Function Definitions

CC_CAN_ERR 			CC_CAN_Init(CAN_HandleTypeDef * can1_, CAN_HandleTypeDef * can2_){
	CC_CAN_ERR ret = CC_CAN_ERR__OK;
	HAL_StatusTypeDef Hret = HAL_OK;

	// Init variables
	RPM_Captured				= 0;
	RPM_BN_Captured				= 0;
	CC_CAN_hcan1 				= can1_;
	CC_CAN_hcan2 				= can2_;


	// Init peripherals
	if( CC_CAN_hcan1->State != HAL_CAN_STATE_RESET ) HAL_CAN_DeInit(CC_CAN_hcan1);
	if( CC_CAN_hcan2->State != HAL_CAN_STATE_RESET ) HAL_CAN_DeInit(CC_CAN_hcan2);

	CC_CAN_hcan1->Instance = CAN1;
	CC_CAN_hcan1->Init.Prescaler = CAN_HS_PRESCALER;
	CC_CAN_hcan1->Init.Mode = CAN_MODE_NORMAL;
	CC_CAN_hcan1->Init.SyncJumpWidth = CAN_SJW_2TQ;
	CC_CAN_hcan1->Init.TimeSeg1 = CAN_BS1_4TQ;
	CC_CAN_hcan1->Init.TimeSeg2 = CAN_BS2_4TQ;
	CC_CAN_hcan1->Init.TimeTriggeredMode = DISABLE;
	CC_CAN_hcan1->Init.AutoBusOff = DISABLE;
	CC_CAN_hcan1->Init.AutoWakeUp = DISABLE;
	CC_CAN_hcan1->Init.AutoRetransmission = ENABLE;
	CC_CAN_hcan1->Init.ReceiveFifoLocked = DISABLE;
	CC_CAN_hcan1->Init.TransmitFifoPriority = DISABLE;
	Hret = HAL_CAN_Init(CC_CAN_hcan1);
	while( Hret != HAL_OK ) __NOP();

	CC_CAN_hcan2->Instance = CAN2;
	CC_CAN_hcan2->Init.Prescaler = CAN_HS_PRESCALER;
	CC_CAN_hcan2->Init.Mode = CAN_MODE_NORMAL;
	CC_CAN_hcan2->Init.SyncJumpWidth = CAN_SJW_2TQ;
	CC_CAN_hcan2->Init.TimeSeg1 = CAN_BS1_4TQ;
	CC_CAN_hcan2->Init.TimeSeg2 = CAN_BS2_4TQ;
	CC_CAN_hcan2->Init.TimeTriggeredMode = DISABLE;
	CC_CAN_hcan2->Init.AutoBusOff = DISABLE;
	CC_CAN_hcan2->Init.AutoWakeUp = DISABLE;
	CC_CAN_hcan2->Init.AutoRetransmission = ENABLE;
	CC_CAN_hcan2->Init.ReceiveFifoLocked = DISABLE;
	CC_CAN_hcan2->Init.TransmitFifoPriority = DISABLE;
	Hret = HAL_CAN_Init(CC_CAN_hcan2);
	while( Hret != HAL_OK ) __NOP();

	// start the peripherals
	CAN_FilterTypeDef sf1;
	sf1.FilterMaskIdHigh = 0x0000;
	sf1.FilterMaskIdLow = 0x0000;
	sf1.FilterIdHigh = 0x0000;
	sf1.FilterIdLow = 0x0000;
	sf1.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	sf1.FilterBank = 0;
	sf1.FilterMode = CAN_FILTERMODE_IDMASK;
	sf1.FilterScale = CAN_FILTERSCALE_32BIT;
	sf1.FilterActivation = CAN_FILTER_ENABLE;
	sf1.SlaveStartFilterBank = 14;
	Hret = HAL_CAN_ConfigFilter(CC_CAN_hcan1, &sf1);
	while( Hret != HAL_OK ) __NOP();
	Hret = HAL_CAN_Start(CC_CAN_hcan1);
	while( Hret != HAL_OK ) __NOP();
	Hret = HAL_CAN_ActivateNotification(CC_CAN_hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
	while( Hret != HAL_OK ) __NOP();


	CAN_FilterTypeDef sf2;
	sf2.FilterMaskIdHigh = 0x0000;
	sf2.FilterMaskIdLow = 0x0000;
	sf2.FilterIdHigh = 0x0000;
	sf2.FilterIdLow = 0x0000;
	sf2.FilterFIFOAssignment = CAN_FILTER_FIFO1;
	sf2.FilterBank = 14;
	sf2.FilterMode = CAN_FILTERMODE_IDMASK;
	sf2.FilterScale = CAN_FILTERSCALE_32BIT;
	sf2.FilterActivation = CAN_FILTER_ENABLE;
	sf2.SlaveStartFilterBank = 14;
	Hret = HAL_CAN_ConfigFilter(CC_CAN_hcan2, &sf2);
	while( Hret != HAL_OK ) __NOP();
	Hret = HAL_CAN_Start(CC_CAN_hcan2);
	while( Hret != HAL_OK ) __NOP();
	Hret = HAL_CAN_ActivateNotification(CC_CAN_hcan2, CAN_IT_RX_FIFO1_MSG_PENDING);
	while( Hret != HAL_OK ) __NOP();

	return ret;
}


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan){
	// Received from CAR side
	HAL_CAN_GetRxMessage(CC_CAN_hcan1, CAN_RX_FIFO0, &(CAN1_RxPacket.Header), CAN1_RxPacket.Data);

	// Forward to CLUSTER
	CAN2_TxPacket.Header.DLC 	= CAN1_RxPacket.Header.DLC;
	CAN2_TxPacket.Header.ExtId 	= CAN1_RxPacket.Header.ExtId;
	CAN2_TxPacket.Header.IDE 	= CAN1_RxPacket.Header.IDE;
	CAN2_TxPacket.Header.RTR 	= CAN1_RxPacket.Header.RTR;
	CAN2_TxPacket.Header.StdId 	= CAN1_RxPacket.Header.StdId;
	for(CAN2_TxPacket_CopyCnt=0; CAN2_TxPacket_CopyCnt<8; ++CAN2_TxPacket_CopyCnt){
		CAN2_TxPacket.Data[CAN2_TxPacket_CopyCnt] = CAN1_RxPacket.Data[CAN2_TxPacket_CopyCnt];
	}

	// Inject data if needed
	if( CAN2_TxPacket.Header.StdId == CAN_ID__BN_RPM ){
		if( RPM_Captured ){
			CC_CAN_InjectRPM(CAN2_TxPacket.Data, &RPM_LastSeen_H, &RPM_LastSeen_L);
		}
		// Store the BN frame data for later repeats
		RPM_BN_Captured = 1;
		CAN2_TxPacket_BNRepeat.Header.DLC 	= CAN1_RxPacket.Header.DLC;
		CAN2_TxPacket_BNRepeat.Header.ExtId = CAN1_RxPacket.Header.ExtId;
		CAN2_TxPacket_BNRepeat.Header.IDE 	= CAN1_RxPacket.Header.IDE;
		CAN2_TxPacket_BNRepeat.Header.RTR 	= CAN1_RxPacket.Header.RTR;
		CAN2_TxPacket_BNRepeat.Header.StdId = CAN1_RxPacket.Header.StdId;
		for(CAN2_TxPacket_CopyCnt=0; CAN2_TxPacket_CopyCnt<8; ++CAN2_TxPacket_CopyCnt){
			CAN2_TxPacket_BNRepeat.Data[CAN2_TxPacket_CopyCnt] = CAN1_RxPacket.Data[CAN2_TxPacket_CopyCnt];
		}
	}

	// Push frame out
	HAL_CAN_AddTxMessage(CC_CAN_hcan2, &(CAN2_TxPacket.Header), (CAN2_TxPacket.Data), &CC_CAN2_TxMailbox);

	// Extract data if present
	if( CAN2_TxPacket.Header.StdId == CAN_ID__BM_RPM ){
		CC_CAN_ExtractRPM(CAN2_TxPacket.Data, &RPM_LastSeen_H, &RPM_LastSeen_L);
		RPM_Captured = 1;
	}

	// Forward to UART
	CC_UART_SendCANFrame(&CAN1_RxPacket, CC_UART_CANDIR__FROMCAR);
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan){
	// Received from CLUSTER side
	HAL_CAN_GetRxMessage(CC_CAN_hcan2, CAN_RX_FIFO1, &(CAN2_RxPacket.Header), CAN2_RxPacket.Data);

	// Forward to CAR
	CAN1_TxPacket.Header.DLC 	= CAN2_RxPacket.Header.DLC;
	CAN1_TxPacket.Header.ExtId 	= CAN2_RxPacket.Header.ExtId;
	CAN1_TxPacket.Header.IDE 	= CAN2_RxPacket.Header.IDE;
	CAN1_TxPacket.Header.RTR 	= CAN2_RxPacket.Header.RTR;
	CAN1_TxPacket.Header.StdId 	= CAN2_RxPacket.Header.StdId;
	for(CAN1_TxPacket_CopyCnt=0; CAN1_TxPacket_CopyCnt<8; ++CAN1_TxPacket_CopyCnt){
		CAN1_TxPacket.Data[CAN1_TxPacket_CopyCnt] = CAN2_RxPacket.Data[CAN1_TxPacket_CopyCnt];
	}

	HAL_CAN_AddTxMessage(CC_CAN_hcan1, &(CAN1_TxPacket.Header), (CAN1_TxPacket.Data), &CC_CAN1_TxMailbox);

	// Forward to UART
	//CC_UART_SendCANFrame(&CAN2_RxPacket, CC_UART_CANDIR__FROMIC);
}

CC_CAN_ERR			CC_CAN1_Send(CAN_TxPacketType * CPacket){
	static HAL_StatusTypeDef ret;
	ret = HAL_CAN_AddTxMessage(CC_CAN_hcan1, &(CPacket->Header), (CPacket->Data), &CC_CAN1_TxMailbox);
	if(ret != HAL_OK) 	return CC_CAN_ERR__FATAL;
	return CC_CAN_ERR__OK;
}

CC_CAN_ERR			CC_CAN2_Send(CAN_TxPacketType * CPacket){
	static HAL_StatusTypeDef ret;
	ret = HAL_CAN_AddTxMessage(CC_CAN_hcan2, &(CPacket->Header), (CPacket->Data), &CC_CAN2_TxMailbox);
	if(ret != HAL_OK) 	return CC_CAN_ERR__FATAL;
	return CC_CAN_ERR__OK;
}

static inline void	CC_CAN_InjectRPM(volatile uint8_t * Data, volatile uint8_t * RPM_H, volatile uint8_t * RPM_L){
	// Inject lower 13 bits at offset 43
	// Example: 07 FF ~ RPM of 4.1k (same example as in ExtractRPM)
	*(Data + 5) = ((*(Data + 5)) & 0xE0 ) | ((*RPM_H) & 0x1F);
	*(Data + 6) = *RPM_L;
}
static inline void	CC_CAN_ExtractRPM(volatile uint8_t * Data, volatile uint8_t * RPM_H, volatile uint8_t * RPM_L){
	// Extract UPPER 13bits at offset 0
	// Example: 3F FF ~ RPM of 4.1k (ALL MSB count), most LSB bit appears not to matter (register value of 2k ~ 4k RPM displayed)
	*RPM_H = ((*(Data + 0)) >> 3);
	*RPM_L = ((*(Data + 0)) << 7) | ((*(Data + 1)) >> 1);
}
static inline void	CC_CAN_InjectRPM_BNStyle(volatile uint8_t * Data, volatile uint8_t * RPM_H, volatile uint8_t * RPM_L){
	// Inject lower 13 bits at offset 43
	// Example: 07 FF ~ RPM of 4.1k (same example as in ExtractRPM)
	*(Data + 5) = ((*(Data + 5)) & 0xE0 ) | ((*RPM_H) & 0x1F);
	*(Data + 6) = *RPM_L;
}
