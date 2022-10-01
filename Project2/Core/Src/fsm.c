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


current_state_t servos_now;
dev_t dev;

int serv_delay [2];

void execute()
{
	startup();
	fetch_inst();
	hold();

}

void startup()
{
	Buf_Init();
	set_states(servos_now, serv_moving, serv_moving);
	serv_delay[0] = get_mov_delay(servos_now, servo1, serv_pos1);
	serv_delay[1] = get_mov_delay(servos_now, servo2, serv_pos1);
	move_servo(1, serv_pos1);
	move_servo(2, serv_pos1);
	HAL_Delay(max(serv_delay));
	set_states(servos_now, serv_pos1, serv_pos1);
}

void fetch_inst()
{

}
current_state_t get_state()
{
	current_state_t now;
	return now;
}

void chk_state()

void set_states(current_state_t *new, int new_state1, int new_state2)
{
	set_state(new, 1, new_state1);
	set_state(new, 2, new_state2);
}

void set_state(current_state_t *new, int serv, int new_state)
{
	if (serv == 1)
	{
		new->servo1 = new_state;
	}
	else if(serv == 2)
	{
		new->servo2 = new_state;
	}
}
