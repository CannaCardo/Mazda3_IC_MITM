/*
 * CC_UART_Handler.h
 *
 *  Created on: Oct 26, 2024
 *      Author: CannaC
 */

#ifndef INC_CC_UART_HANDLER_H_
#define INC_CC_UART_HANDLER_H_



// ---------------------- Files

#include "stm32f4xx_hal.h"
#include "CC_CAN_Handler.h"

// ---------------------- Type Definitions

typedef enum{
	CC_UART_ERR__OK,
	CC_UART_ERR__BUSY,
	CC_UART_ERR__FATAL,
} CC_UART_ERR;

typedef enum{
	CC_UART_CANDIR__FROMCAR,
	CC_UART_CANDIR__FROMIC,
} CC_UART_CANDIR;



// ---------------------- Function Declarations

CC_UART_ERR 	CC_UART_Init(UART_HandleTypeDef * huart_, TIM_HandleTypeDef * htim_);
void			CC_UART_TimIRQ(void);
void			CC_UART_SendCANFrame(CAN_RxPacketType * CPacket, CC_UART_CANDIR Dir);
CC_UART_ERR 	CC_UART_SetDebug(uint8_t Enable);

#endif /* INC_CC_UART_HANDLER_H_ */
