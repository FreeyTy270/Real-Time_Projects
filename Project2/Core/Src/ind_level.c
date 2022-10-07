/*
 * ind_level.c
 *
 *  Created on: Oct 7, 2022
 *      Author: Ty Freeman
 *
 *      File for all functions related to individual servo motors. The middle layer section
 */

#include "stm32l4xx.h"
#include "ind_level.h"
#include "low_level.h"

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

void go(servo_t *servo)
{
	int delay = 0;
	opcode_t command;
	command.operation = MOV;
	command.data = 1;

	servo->position = serv_pos5;
	servo->status = status_running;
	delay = run_inst(servo->dev, serv_pos1, command);
	HAL_Delay(delay);
	servo->position = serv_pos1;
	servo->status = status_paused;
	servo->recipe_index = 0;
	servo->nxt_event = stop;
	servo->loop_flg = 0;
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
			servo->status = status_running;
		}
		else if(servo->nxt_event == right && servo->position < 5)
		{
			servo->status = status_running;
		}
		else
		{
			servo->status = status_cmd_error;
		}
	}
	else
	{
		servo->status = status_cmd_error;
	}


	if(servo->status == status_cmd_error)
	{
		response = 1;
		if(servo->dev == 1)
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0);
		}
	}
	else if(servo->status == status_paused)
	{
		response = 1;
		if(servo->dev == 1)
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_6, 0);
		}

	}
	else if(servo->status == status_running)
	{
		response = 0;

		if(servo->dev == 1)
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);
		}

		if(user_mov)
		{
			servo->status = status_paused;
		}
	}

	return response;
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
					servo->status = status_loop_error;
					servo->new_com.operation = WAIT;
					servo->new_com.data = 0;
					if(servo->dev == 1)
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
int run_servo(servo_t *servo)
{
	int delay = run_inst(servo->dev, servo->position, servo->new_com);

	chk_delay(servo, delay);

	return delay;

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

void swap(servo_t *servo, servo_t *servo, int swap1, int swap2)
{
	uint8_t *temp;

	if(swap1)
	{

	}



		if(system->servo1->nxt_event == swap || system->servo2->nxt_event == swap)
		{
			temp = system->servo1->recipe;
			system->servo1->recipe = system->servo2->recipe;
			system->servo2->recipe = temp;
		}
}

void hold(servo_t *servo)
{
	servo->old_com = servo->new_com;

	servo->new_com.operation = WAIT;
	servo->new_com.data = 0;
}
