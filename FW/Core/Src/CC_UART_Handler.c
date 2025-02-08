/*
 * CC_UART_Handler.c
 *
 *  Created on: Oct 26, 2024
 *      Author: CannaC
 */


/*
 * This UART Handler works as follows:
 * 		a circular buffer of uint8_t[1+4+1+8] (Dir + ID + Length + CAN DATA) stores received CAN frames
 *		TIM2 IRQ checks this buffer, and triggers a UART TX if non-empty (sends a single entry)
 *		Once sent, UART TX Cmplt IRQ checks if any more are still present - if so, keeps sending
 * */


// ---------------------- Files

#include "CC_UART_Handler.h"
#include "CC_UART_Handler_priv.h"
#include "CC_CAN_Handler.h"
#include "CC_LED_Handler.h"
#include "CC_CircBuffer.h"
#include "stm32f4xx_hal.h"

#include <stdlib.h>

// ---------------------- Local Variables

CC_UART_Type 			CC_UART;
UART_HandleTypeDef * 	CC_UART_huart;
TIM_HandleTypeDef *		CC_UART_htim;

// ---------------------- Helper Function Declarations

uint8_t CC_UART_BUFTX_SEMAPHORE_CheckAndSet( void ){
	static volatile uint32_t 	prim;
	static volatile uint8_t 	value;

	prim = __get_PRIMASK();
	__disable_irq();

	value = CC_UART.BufTx_Busy;
	CC_UART.BufTx_Busy = 1;

	if (!prim){
	    __enable_irq();
	}

	return value;
}

// ---------------------- Function Definitions

CC_UART_ERR 			CC_UART_Init(UART_HandleTypeDef * huart_, TIM_HandleTypeDef * htim_){
	CC_UART_ERR ret = CC_UART_ERR__OK;

	// Init variables
	CC_UART_huart 				= huart_;
	CC_UART_htim  				= htim_;
	CC_UART.BufTx_Busy			= 0;
	//CC_UART.Debug_Enabled 		= 0; // value is set by LED handler, no need

	// Allocate memory
	CC_UART.BufCAN = CC_CircBuffer_Init(32, sizeof(uint8_t) * CC_UART_CIRCBUFFER_ARRAY_LEN);

	// Init peripherals
	// USART huart (done in main for now, baud 115200)

	// TIMER 2 - PARSER
	// TIM 1/9/10/11 - APB2 (180MHz), rest: APB1 (90MHz)
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	//
	HAL_TIM_Base_DeInit(CC_UART_htim);
	CC_UART_htim->Init.Prescaler 			= 90-1; //1us per tick
	CC_UART_htim->Init.CounterMode 			= TIM_COUNTERMODE_UP;
	CC_UART_htim->Init.Period 				= 10000-1; // 10ms
	CC_UART_htim->Init.ClockDivision 		= TIM_CLOCKDIVISION_DIV1;
	CC_UART_htim->Init.AutoReloadPreload 	= TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(CC_UART_htim);
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(CC_UART_htim, &sClockSourceConfig);
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(CC_UART_htim, &sMasterConfig);
	HAL_TIM_Base_Start_IT(CC_UART_htim);

	// Kickstart receive
	HAL_UART_Receive_IT(CC_UART_huart, &(CC_UART.BufRx_filter), 1);

	return ret;
}

CC_UART_ERR 			CC_UART_SetDebug(uint8_t Enable){
	static CC_UART_ERR ret;
	ret = CC_UART_ERR__OK;

	if( Enable == 0 ){
		CC_UART.Debug_Enabled = 0;
	}else if( Enable == 1 ){
		CC_UART.Debug_Enabled = 1;
	}else{
		ret = CC_UART_ERR__FATAL;
	}

	return ret;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	// Discarded in this version
	HAL_UART_Receive_IT(CC_UART_huart, &(CC_UART.BufRx_filter), 1);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	// Free CircBuffer resources

	if( CC_UART.BufCAN->Buf_Len > 0 ){
	if( CC_CircBuffer_Remove(CC_UART.BufCAN, 0, 1) != CC_CIRCBUFFER_ERR__OK ){
		// Set LED to signal underflow
		CC_LED_Set(CC_LED1, CC_LEDSTATE_ON);
	}
	}

	if( CC_UART.BufCAN->Buf_Len > 0 ){
		// Keep sending
		HAL_UART_Transmit_IT(CC_UART_huart, (const uint8_t*)(CC_CircBuffer_GetPtr(CC_UART.BufCAN, 0)), CC_UART_CIRCBUFFER_ARRAY_LEN);
	}else{
		CC_UART.BufTx_Busy = 0;
	}
}

void CC_UART_TimIRQ(void){
	// This irq shall be called every 10ms, LOW priority

	if( CC_UART_BUFTX_SEMAPHORE_CheckAndSet() ){
		// UART BUSY
	}else{
		if( CC_UART.BufCAN->Buf_Len > 0 ){
			// Start sending
			HAL_UART_Transmit_IT(CC_UART_huart, (const uint8_t*)(CC_CircBuffer_GetPtr(CC_UART.BufCAN, 0)), CC_UART_CIRCBUFFER_ARRAY_LEN);
		}else{
			CC_UART.BufTx_Busy = 0;
		}
	}
}

void	CC_UART_SendCANFrame(CAN_RxPacketType * CPacket, CC_UART_CANDIR Dir){
	static uint8_t i;
	static uint8_t pos;
	static uint32_t currTime;

	// Direction
	pos = 0;
	CC_UART.BufCAN_TempFrame[  pos] = Dir;
	// TimeStamp
	currTime = HAL_GetTick();
	CC_UART.BufCAN_TempFrame[++pos] = ((currTime) & 0xFF000000) >> 24;
	CC_UART.BufCAN_TempFrame[++pos] = ((currTime) & 0x00FF0000) >> 16;
	CC_UART.BufCAN_TempFrame[++pos] = ((currTime) & 0x0000FF00) >> 8;
	CC_UART.BufCAN_TempFrame[++pos] = ((currTime) & 0x000000FF) >> 0;
	// ID
	CC_UART.BufCAN_TempFrame[++pos] = ((CPacket->Header.StdId) & 0xFF000000) >> 24;
	CC_UART.BufCAN_TempFrame[++pos] = ((CPacket->Header.StdId) & 0x00FF0000) >> 16;
	CC_UART.BufCAN_TempFrame[++pos] = ((CPacket->Header.StdId) & 0x0000FF00) >> 8;
	CC_UART.BufCAN_TempFrame[++pos] = ((CPacket->Header.StdId) & 0x000000FF) >> 0;
	// Length
	CC_UART.BufCAN_TempFrame[++pos] = CPacket->Header.DLC;
	// Data
	for(i=0; i<8; ++i){
		CC_UART.BufCAN_TempFrame[++pos] = CPacket->Data[i];
	}
	if( CC_UART.Debug_Enabled ){
		if( CC_CircBuffer_Push( CC_UART.BufCAN, CC_UART.BufCAN_TempFrame ) != CC_CIRCBUFFER_ERR__OK ){
			// Set LED to signal overflow
			CC_LED_Set(CC_LED2, CC_LEDSTATE_ON);
		}
	}
}

