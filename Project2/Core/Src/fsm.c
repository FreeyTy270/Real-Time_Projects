/*
 * fsm.c
 *
 *  Created on: Sep 28, 2022
 *      Author: Ty Freeman
 */

#include <stdio.h>
#include <stdlib.h>
#include "fsm.h"
#include "uart.h"
#include "servo.h"
#include "command.h"
#include "stm32l4xx.h"

int servo_delay [2];

void execute(int flg)
{
	servo_t servo1 = {1, serv_unknown, recipes[0]};
	servo_t servo2 = {2, serv_unknown, recipes[1]};
	system_state_t sys = {&servo1, &servo2};

	if(!flg)
	{
		startup(&sys);
		flg = 1;
	}
	fetch_next_sys(&sys);
	run_next(&sys);

}

void startup(system_state_t *now)
{
	Buf_Init();
	now->servo1->recipe = recipes[0];
	now->servo2->recipe = recipes[1];
	set_states(now, serv_moving, serv_moving);
	servo_delay[0] = get_mov_delay((now->servo1->position - serv_pos1));
	servo_delay[1] = get_mov_delay((now->servo2->position - serv_pos1));
	move_servo(now->servo1->dev, serv_pos1);
	move_servo(now->servo1->dev, serv_pos1);
	HAL_Delay(max(servo_delay[0], servo_delay[1]));
	set_states(now, serv_pos1, serv_pos1);
}

void fetch_next_sys(system_state_t *system)
{
	fetch_next(system->servo1);
	fetch_next(system->servo2);
}

void fetch_next(servo_t *servo)
{
	int loop_start = 0;
	int nrepeat = 0;

	static int new_index = 0;

	switch(servo->loop_flg)
		{
			case 0:
				servo->old_com = servo->new_com;
				servo->new_com = read_recipe(servo->recipe, servo->recipe_index);
				break;
			case 1:
				servo->loop_flg = 0;
				loop_start = servo->recipe_index + 1;
				nrepeat = servo->new_com.data;
				new_index = loop_start;
				break;
			case 2:
				if(chk_loop(nrepeat) == 0)
				{
					servo->loop_flg = 0;
					break;
				}
				servo->loop_flg += 1;
			case 3:
				servo->new_com = read_recipe(servo->recipe, new_index);
				new_index++;
				break;
		}
}

void run_next(system_state_t *system)
{
	servo_delay[0] = run_inst(system->servo1->dev, system->servo1->position, system->servo1->new_com);
	servo_delay[1] = run_inst(system->servo2->dev, system->servo2->position, system->servo2->new_com);

	if(servo_delay[0] > 31)
	{
		switch(servo_delay[0])
		{
		case 32:
			system->servo1->loop_flg = 1;
			break;
		case 33:
			system->servo1->loop_flg = 2;
		case 34:
			system->servo1->position = recipe_ended;
		}
	}
	HAL_Delay(100 + max(servo_delay[0], servo_delay[1]));
}

void set_states(system_state_t *new, int new_state1, int new_state2)
{
	set_state(new->servo1, new_state1);
	set_state(new->servo2, new_state2);
}

void set_state(servo_t *serv, int new_state)
{
	serv->position = new_state;
}

uint8_t max(uint8_t num1, uint8_t num2)
{
	int n = ((num1-num2) > 0) ? num1 : num2;
	return n;
}
