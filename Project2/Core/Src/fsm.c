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
	fetch_inst(&sys.servo1);
	fetch_inst(&sys.servo2);
	run_inst(&sys);
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

void fetch_next(system_state_t *system)
{
	static int serv1_repeat = 0;
	static int serv2_repeat = 0;
	system->servo1->old_com = system->servo1->new_com;
	system->servo2->old_com = system->servo2->new_com;

	system->servo1->new_com = read_recipe(system->servo1->recipe, system->servo1->recipe_index);
	system->servo2->new_com = read_recipe(system->servo2->recipe, system->servo2->recipe_index);

	serv1_repeat = chk_loop(system->servo1);
	serv2_repeat = chk_loop(system->servo2);

	if(serv1_repeat > 0 || serv2_repeat > 0)
	{

	}
}

void run_next(system_state_t *system)
{
	run_inst(system->servo1);
	run_inst(system->servo2);
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
