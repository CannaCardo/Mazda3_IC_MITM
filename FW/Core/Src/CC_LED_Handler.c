/*
 * CC_LED_Handler.c
 *
 *  Created on: Oct 12, 2024
 *      Author: CannaC
 */


// ---------------------- Files

#include "main.h"
#include "CC_LED_Handler.h"
#include "CC_UART_Handler.h"
#include "stm32f4xx_hal.h"
#include <stdlib.h>

// ---------------------- Local Variables

uint8_t 			CC_LED_Enable;
CC_LED_STATE 		CC_LED_State[3];
GPIO_TypeDef*  		CC_LED_Port[3];
uint16_t 			CC_LED_Pin[3];
TIM_HandleTypeDef*	CC_LED_htim;

// ---------------------- Helper Function Declarations


// ---------------------- Function Definitions

CC_LED_ERR 		CC_LED_Init(TIM_HandleTypeDef* htim){
	CC_LED_ERR ret = CC_LED_ERR__OK;

	CC_LED_Enable  = 0;
	CC_LED_Port[0] = LED1_GPIO_Port;
	CC_LED_Pin[0]  = LED1_Pin;
	CC_LED_Port[1] = LED2_GPIO_Port;
	CC_LED_Pin[1]  = LED2_Pin;
	CC_LED_Port[2] = LED3_GPIO_Port;
	CC_LED_Pin[2]  = LED3_Pin;
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
	CC_LED_State[0] = CC_LEDSTATE_OFF;
	CC_LED_State[1] = CC_LEDSTATE_OFF;
	CC_LED_State[2] = CC_LEDSTATE_OFF;
	CC_UART_SetDebug( CC_LED_Enable ); // Initializes debug flag


	// Init timer, 10ms tick (TIM1)
	// TIM 1/9/10/11 - APB2 (180MHz), rest: APB1 (90MHz)
	CC_LED_htim = htim;
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	HAL_TIM_Base_DeInit(CC_LED_htim);
	CC_LED_htim->Init.Prescaler 			= 180-1; //1us per tick
	CC_LED_htim->Init.CounterMode 			= TIM_COUNTERMODE_UP;
	CC_LED_htim->Init.Period 				= 10000-1; // 10ms
	CC_LED_htim->Init.ClockDivision 		= TIM_CLOCKDIVISION_DIV1;
	CC_LED_htim->Init.AutoReloadPreload 	= TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(CC_LED_htim);
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(CC_LED_htim, &sClockSourceConfig);
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(CC_LED_htim, &sMasterConfig);
	HAL_TIM_Base_Start_IT(CC_LED_htim);

	return ret;
}

void			CC_LED_TimIRQ(void){ // 10ms period
	static uint32_t cnt = 0;

	CC_LED_CheckEnable();
	++cnt;
	if( cnt >= 80){ //200ms on, 800ms off
		CC_LED_Set(CC_LED0, CC_LEDSTATE_ON);
	}else{
		CC_LED_Set(CC_LED0, CC_LEDSTATE_OFF);
	}
	if( cnt >= 100){ // 1000ms
		cnt = 0;
	}
}

CC_LED_ERR		CC_LED_CheckEnable(void){ // 10ms period
	static CC_LED_ERR ret;
	static uint8_t i;
	static uint8_t  cnt_debouncing = 0;
	ret = CC_LED_ERR__OK;

	if (cnt_debouncing > 0){
		--cnt_debouncing;
	}else{
		if (HAL_GPIO_ReadPin(LED_EN_GPIO_Port, LED_EN_Pin) == GPIO_PIN_RESET){
			if( CC_LED_Enable == 1 ){
				cnt_debouncing = 20; // 200ms de-bounce time
				CC_LED_Enable = 0;
				CC_UART_SetDebug( CC_LED_Enable );
			}
			for(i=0; i<3; ++i){
				HAL_GPIO_WritePin(CC_LED_Port[i], CC_LED_Pin[i], GPIO_PIN_RESET);
			}
		}else{
			if( CC_LED_Enable == 0 ){
				cnt_debouncing = 20; // 200ms de-bounce time
				CC_LED_Enable = 1;
				CC_UART_SetDebug( CC_LED_Enable );
			}
			for(i=0; i<3; ++i){
				if( CC_LED_State[i] == CC_LEDSTATE_ON ){
					HAL_GPIO_WritePin(CC_LED_Port[i], CC_LED_Pin[i], GPIO_PIN_SET);
				}else{
					HAL_GPIO_WritePin(CC_LED_Port[i], CC_LED_Pin[i], GPIO_PIN_RESET);
				}
			}
		}
	}
	return ret;
}

CC_LED_ERR 		CC_LED_Set(CC_LED_PERIPH LED, CC_LED_STATE State){
	static CC_LED_ERR ret;
	ret = CC_LED_ERR__OK;

	if( LED > CC_LED2) return CC_LED_ERR__FATAL;
	if( State > CC_LEDSTATE_OFF) return CC_LED_ERR__FATAL;

	CC_LED_State[LED] = State;
	if( CC_LED_Enable ){
		if( CC_LED_State[LED] == CC_LEDSTATE_ON ){
			HAL_GPIO_WritePin(CC_LED_Port[LED], CC_LED_Pin[LED], GPIO_PIN_SET);
		}else{
			HAL_GPIO_WritePin(CC_LED_Port[LED], CC_LED_Pin[LED], GPIO_PIN_RESET);
		}
	}

	return ret;
}
