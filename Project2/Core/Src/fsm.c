/*
 * fsm.c
 *
 *  Created on: Sep 28, 2022
 *      Author: Ty Freeman
 */

#include <stdio.h>
#include <stdlib.h>
#include "fsm.h"
#include "data.h"
#include "uart.h"
#include "servo.h"
#include "stm32l4xx.h"

int servo_delay [2];

extern TIM_HandleTypeDef htim6;
extern uint8_t rxbuf;
extern uint8_t mainbuf[];
extern uint8_t *recipes[];
extern _Bool cr_flg;


typedef struct servo
{
	int dev;
	int position;
	int status;
	int nxt_event;
	uint8_t *recipe;
	int recipe_index;
	opcode_t old_com;
	opcode_t new_com;
	int loop_flg : 3;
}servo_t;

typedef struct system_state
{
	servo_t *servo1;
	servo_t *servo2;

}system_state_t;

void execute(_Bool *flg)
{
	int resp = 0;
	static int delay = 0;

	static servo_t servo1 = {1, serv_unknown, status_paused};
	static servo_t servo2 = {2, serv_unknown, status_paused};
	static system_state_t sys = {&servo1, &servo2};

	HAL_UARTEx_ReceiveToIdle_DMA(&UART, &rxbuf, 1);
	__HAL_DMA_DISABLE_IT(&DMA, DMA_IT_HT);

	if(cr_flg)
	{
		override_process(sys.servo1, mainbuf[0]);
		override_process(sys.servo2, mainbuf[1]);
	}

	if(TIM6->CNT == (1000 + delay) || !*flg)
	{
		if(!*flg)
		{
			startup(&sys);
			*flg = 1;
		}

		resp = chk_states(&sys);
		fetch_next_sys(&sys, resp);
		delay = run_next(&sys);
		TIM6->CNT = 0;
	}

}

void override_process(servo_t *servo, uint8_t cmd)
{
	switch(cmd)
	{
	case 'N':
		break;
	case 'C':
		servo->nxt_event = cont;
		break;
	case 'R':
		servo->nxt_event = right;
		break;
	case 'L':
		servo->nxt_event = left;
		break;
	case 'P':
		servo->nxt_event = stop;
		break;
	case 'B':
		servo->nxt_event = start;
		break;
	case 'S':
		servo->nxt_event = swap;
	}
}

void startup(system_state_t *now)
{
	Buf_Init();
	now->servo1->recipe = recipes[0];
	now->servo2->recipe = recipes[1];
	now->servo1->position = serv_moving;
	now->servo2->position = serv_moving;
	servo_delay[0] = get_mov_delay((serv_pos5 - serv_pos1));
	servo_delay[1] = servo_delay[0];
	move_servo(now->servo1->dev, serv_pos1);
	move_servo(now->servo1->dev, serv_pos1);
	now->servo1->position = serv_pos1;
	now->servo2->position = serv_pos1;
}

int chk_states(system_state_t *system)
{

	int servo1_resp = 0;
	int servo2_resp = 0;
	uint8_t *temp;

	if(system->servo1->nxt_event == swap || system->servo2->nxt_event == swap)
	{
		temp = system->servo1->recipe;
		system->servo1->recipe = system->servo2->recipe;
		system->servo2->recipe = temp;
	}

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

int chk_state(servo_t *servo)
{
	_Bool response = 0;
	_Bool user_mov = 0;

	if(servo->nxt_event == left || servo->nxt_event == right)
	{
		user_mov = 1;
	}

	if(servo->nxt_event == start)
	{
		servo->status = status_running;
		servo->recipe_index = 0;
		servo->nxt_event = cont;
	}
	else if(servo->nxt_event == stop && servo->status == status_running && servo->position != recipe_ended)
	{
		servo->status = status_paused;
	}
	else if(servo->nxt_event == cont && servo->status == status_paused && servo->position != recipe_ended)
	{
		servo->status = status_running;
	}
	else if(user_mov && servo->status == status_paused)
	{
		if(servo->nxt_event == left && servo->position > 1)
		{
			servo->status == status_running;
		}
		else if(servo->nxt_event == right && servo->position < 5)
		{
			servo->status == status_running;
		}
		else
		{
			servo->status == status_cmd_error;
		}
	}
	else
	{
		servo->status = status_cmd_error;
	}


	if(servo->status == status_cmd_error)
	{
		response = 1;
		if(servo->dev = 1)
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
		}
	}
	else if(servo->status == status_paused)
	{
		response = 1;
		if(servo->dev = 1)
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_6, 0);
		}

	}
	else if(servo->status == status_running)
	{
		response = 0;

		if(servo->dev = 1)
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1);
		}

		if(user_mov)
		{
			servo->status = status_paused;
		}
	}

	return response;
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

void fetch_next(servo_t *servo)
{
	int loop_start = 0;
	static int nrepeat = 0;

	static int new_index = 0;

	if(servo->nxt_event == left)
	{
		servo->old_com = servo->new_com;
		servo->new_com.operation = MOV;
		servo->new_com.data = (servo->position - 1);
	}
	else if(servo->nxt_event == right)
	{
		servo->old_com = servo->new_com;
		servo->new_com.operation = MOV;
		servo->new_com.data = (servo->position + 1);
	}
	else
	{
		switch(servo->loop_flg)
		{
			case 0:
				servo->old_com = servo->new_com;
				servo->new_com = read_recipe(servo->recipe, servo->recipe_index);
				servo->recipe_index++;
				break;
			case 1:
				servo->loop_flg = 0;
				loop_start = servo->recipe_index + 1;
				nrepeat = servo->new_com.data;
				new_index = loop_start;
				break;
			case 2:
				servo->old_com = servo->new_com;
				servo->new_com = read_recipe(servo->recipe, servo->recipe_index);
				if(servo->new_com.operation == LOOP)
				{
					servo->status == status_loop_error;
					servo->new_com.operation = WAIT;
					servo->new_com.data = 0;
					if(servo->dev = 1)
					{
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_6, 1);
					}
				}
				servo->recipe_index++;
			case 3:
				if(chk_loop(nrepeat) == 0)
				{
					servo->loop_flg = 0;
					break;
				}
				servo->loop_flg += 1;
			case 4:
				servo->new_com = read_recipe(servo->recipe, new_index);
				new_index++;
				break;
		}
	}
}

int run_next(system_state_t *system)
{
	int delay = 0;

	servo_delay[0] = run_inst(system->servo1->dev, system->servo1->position, system->servo1->new_com);
	servo_delay[1] = run_inst(system->servo2->dev, system->servo2->position, system->servo2->new_com);

	chk_delay(system->servo1, servo_delay[0]);
	chk_delay(system->servo2, servo_delay[1]);

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
}

void chk_delay(servo_t *servo, const int delay)
{
	if(delay > 31 && delay < 35)
		{
			switch(delay)
			{
			case 32:
				servo->loop_flg = 1;
				break;
			case 33:
				servo->loop_flg = 3;
				break;
			case 34:
				servo->position = recipe_ended;
				break;
			default:
				servo->position = serv_moving;
			}
		}
}



void hold(servo_t *servo)
{
	servo->old_com = servo->new_com;

	servo->new_com.operation = WAIT;
	servo->new_com.data = 0;
}

int max(int num1, int num2)
{
	int n = ((num1-num2) > 0) ? num1 : num2;
	return n;
}
