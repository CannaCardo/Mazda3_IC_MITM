/*
 * CC_CAN_Handler_priv.h
 *
 *  Created on: Sep 1, 2024
 *      Author: CannaC
 */

#ifndef INC_CC_CAN_HANDLER_PRIV_H_
#define INC_CC_CAN_HANDLER_PRIV_H_


#include "CC_CAN_Handler.h"


#define CAN_MODE__DRIVER



#define	CAN_ID__BM_RPM	(0x202)
#define	CAN_ID__BN_RPM	(0x130)


#define CAN_HS_PRESCALER		(10)  // 500 000 baud
#define CAN_MS_PRESCALER		(40)  // 125 000 baud


#endif /* INC_CC_CAN_HANDLER_PRIV_H_ */
