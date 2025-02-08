/*
 * CC_LED_Handler.h
 *
 *  Created on: Oct 12, 2024
 *      Author: CannaC
 */

#ifndef INC_CC_LED_HANDLER_H_
#define INC_CC_LED_HANDLER_H_


// ---------------------- Files

#include <stdint.h>
#include "stm32f4xx_hal.h"

// ---------------------- Type Definitions

typedef enum{
	CC_LED_ERR__OK,
	CC_LED_ERR__FATAL,
} CC_LED_ERR;

typedef enum{
	CC_LEDSTATE_ON,
	CC_LEDSTATE_OFF,
} CC_LED_STATE;

typedef enum{
	CC_LED0,
	CC_LED1,
	CC_LED2,
} CC_LED_PERIPH;

// ---------------------- Function Declarations

CC_LED_ERR 		CC_LED_Init(TIM_HandleTypeDef* htim);
CC_LED_ERR		CC_LED_CheckEnable(void);
CC_LED_ERR 		CC_LED_Set(CC_LED_PERIPH LED, CC_LED_STATE State);

void			CC_LED_TimIRQ(void);



#endif /* INC_CC_LED_HANDLER_H_ */
