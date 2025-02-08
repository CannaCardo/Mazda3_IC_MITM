/*
 * CC_UART_Handler_priv.h
 *
 *  Created on: May 23, 2024
 *      Author: CannaC
 */

#ifndef INC_CC_UART_HANDLER_PRIV_H_
#define INC_CC_UART_HANDLER_PRIV_H_


// ---------------------- Files

#include <stdio.h>
#include <stdint.h>
#include "CC_UART_Handler.h"
#include "CC_CircBuffer.h"

// ---------------------- Macros

#define	CC_UART_CIRCBUFFER_ARRAY_LEN	(18) //uint8_t[1+4+4+1+8] (Dir + Timestamp + ID + Length + CAN DATA)

// ---------------------- Type Definitions

typedef struct{
	CC_CIRCBUFFER_TYPE	*	BufCAN;
	uint8_t					BufTx_Busy;
	uint8_t					BufRx_filter;

	uint8_t					BufCAN_TempFrame[CC_UART_CIRCBUFFER_ARRAY_LEN]; // used for copying

	uint8_t					Debug_Enabled; // toggled with LED jumper, enables CAN printout
} CC_UART_Type;



#endif /* INC_CC_UART_HANDLER_PRIV_H_ */
