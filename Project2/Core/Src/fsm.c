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
#include "data.h"
#include "servo.h"
#include "command.h"

int serv_delay [2];

_Bool recipe_end = 0;

void execute()
{
	servo_t servo1 = {1, serv_unknown, recipes[0]};
	servo_t servo2 = {2, serv_unknown, recipes[1]};
	system_state_t sys = {&servo1, &servo2};
	opcode_t com;

	startup(&sys);
	fetch_inst(&sys, com);
	run_inst(&sys, com);
	hold();

}

void startup(system_state_t *now)
{
	Buf_Init();
	now->servo1->recipe = recipe[0];
	now->servo2->recipe = recipe[1];
	set_states(now, serv_moving, serv_moving);
	serv_delay[0] = get_mov_delay(now, servo1, serv_pos1);
	serv_delay[1] = get_mov_delay(now, servo2, serv_pos1);
	move_servo(now->servo1->dev, serv_pos1);
	move_servo(now->servo1->dev, serv_pos1);
	HAL_Delay(max(serv_delay));
	set_states(now, serv_pos1, serv_pos1);
}

void run_inst(system_state_t *system, opcode_t *command)
{
	switch(command->operation)
	{
	case MOV:
		set_state(system->servo1, serv_moving);

	}
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
