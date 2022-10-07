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

typedef struct system_state
{
	servo_t *servo1;
	servo_t *servo2;

}system_state_t;

void execute(_Bool *flg)
{
	int resp = 0;
	static int delay = 0;

	static system_state_t sys = {&servo1, &servo2};

	//TIM6->CNT = 0;
	HAL_UARTEx_ReceiveToIdle_DMA(&UART, &rxbuf, 1);
	__HAL_DMA_DISABLE_IT(&DMA, DMA_IT_HT);

	if(!*flg)
	{
		startup(&sys);
		*flg = 1;
	}

	if(cr_flg)
	{
		override_process(sys.servo1, mainbuf[0]);
		override_process(sys.servo2, mainbuf[1]);
		memset(mainbuf, 0, 2);
		cr_flg = 0;
	}

	resp = chk_states(&sys);
	sys_fetch_next(&sys, resp);
	delay = run_next(&sys);
	HAL_Delay(100 + delay);
}

void startup(system_state_t *system)
{
	Buf_Init();
	go(system->servo1, system->servo2);
}

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

void fetch_next_sys(system_state_t *system, int flg)
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


int max(int num1, int num2)
{
	int n = ((num1-num2) > 0) ? num1 : num2;
	return n;
}
