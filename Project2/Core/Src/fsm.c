/*
 * fsm.c
 *
 *  Created on: Sep 28, 2022
 *      Author: Ty Freeman
 */

#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "stm32l4xx.h"
#include "fsm.h"
#include "ind_level.h"
#include "uart.h"

int servo_delay [2];

extern servo_t servo1;
extern servo_t servo2;
extern uint8_t rxbuf;
extern uint8_t mainbuf[];
extern uint8_t *recipes[];
extern _Bool cr_flg;

/*
 * Container definition of whole system. Pointers to the defined devices: servo1 and servo2
 */
typedef struct system_state
{
	servo_t *servo1;
	servo_t *servo2;

}system_state_t;

/******************************************************************************************
* 																						  *
** Program function. Flg parameter signals very beginning of program or repeat iteration **
* 																						  *
******************************************************************************************/
void execute(_Bool *flg)
{
	int resp = 0;
	static int delay = 0;

	static system_state_t sys = {&servo1, &servo2}; // Declare system

	HAL_UARTEx_ReceiveToIdle_DMA(&UART, &rxbuf, 1); // Begin DMA
	__HAL_DMA_DISABLE_IT(&DMA, DMA_IT_HT);

	if(!*flg) // If the flg is not set then this is the first time execute is called and the servos must be initialized
	{
		startup(&sys);
		*flg = 1;
	}

	set_servo(sys.servo1, p_nxt_event, nothing);
	set_servo(sys.servo2, p_nxt_event, nothing);

	if(cr_flg) // If carriage return is seen from DMA then first check the entered overrides
	{
		override_process(sys.servo1, mainbuf[0]);
		override_process(sys.servo2, mainbuf[1]);
		memset(mainbuf, 0, 2); // Clear the command buffer for future reads
		cr_flg = 0; // Clear carriage return flag
	}

	resp = chk_states(&sys); // Gather response of each servo. Check/set next state and process next event requests
	sys_fetch_next(&sys, resp); // Based on servo responses fetch the next command for each servo
	delay = run_next(&sys); // Process the previously set command. MOV or return modified delay value
	HAL_Delay(100 + delay); // Hold the rest of the time for the desired 100ms loop frequency
}

/***************************************************************************
* 																		   *
** Call initialization of both servos and the uart initialization as well **
* 																		   *
***************************************************************************/
void startup(system_state_t *system)
{
	Buf_Init();
	go(system->servo1);
	go(system->servo2);
}

/************************************************************
* 															*
** Call the state checking routine defined in ind_level.c. **
** If servo status is running the chk_state response is 0  **
** If the servo is stopped for another reason it will be   **
** 					greater than zero					   **
* 															*
************************************************************/
int chk_states(system_state_t *system)
{

	int servo1_resp = 0;
	int servo2_resp = 0;


	if(chk_state(system->servo1) > 0)
	{
		servo1_resp = 1;
	}
	if(chk_state(system->servo2) > 0)
	{
		servo2_resp = 2;
	}

	return servo1_resp + servo2_resp;
}

/****************************************************************
* 																*
** Given the response from chk_states this routine will either **
** hold the necessary servo[s] or fetch the next command to be **
** 							executed 						   **
* 																*
****************************************************************/
void sys_fetch_next(system_state_t *system, int flg)
{
	switch(flg)
		{
		case 1:
			hold(system->servo1);
			break;
		case 2:
			hold(system->servo2);
			break;
		case 3:
			hold(system->servo1);
			hold(system->servo2);
			break;
		default:
			fetch_next(system->servo1);
			fetch_next(system->servo2);
		}
}

/**********************************************************
* 														  *
** Run the commands fetched from sys_fetch_next. Process **
**  returned delay to return the correct delay modifier  **
* 														  *
**********************************************************/
int run_next(system_state_t *system)
{
	int delay = 0;

	servo_delay[0] = run_servo(system->servo1);
	servo_delay[1] = run_servo(system->servo2);

	if(servo_delay[0] > 31)
	{
		delay = servo_delay[1];
	}
	else if(servo_delay[1] > 31)
	{
		delay = servo_delay[0];
	}
	else if(servo_delay[0] > 31 && servo_delay[1] > 31)
	{
		delay = 0;
	}
	else
	{
		delay = max(servo_delay[0], servo_delay[1]);
	}
	return delay;
}

/**********************************
* 							      *
** Returns maximum of two values **
* 								  *
**********************************/
int max(int num1, int num2)
{
	int n = ((num1-num2) > 0) ? num1 : num2;
	return n;
}
