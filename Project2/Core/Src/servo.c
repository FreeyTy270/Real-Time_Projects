/*
 * servo.c
 *
 *  Created on: Sep 28, 2022
 *      Author: Ty Freeman
 */
#include "servo.h"
#include "fsm.h"
#include "data.h"

#define MOV_TIME	75

extern TIM_HandleTypeDef htim2;

static typedef enum
{
	pos1 = 50,
	pos2 = 63,
	pos3 = 75,
	pos4 = 88,
	pos5 = 100
}position;


int get_mov_delay(current_state_t *now, int serv, servo_state newpos)
{
	int travel_time = 0;
	int delta = 0;

	if(serv == 1)
	{
		delta = abs(newpos - now->servo1);
	}
	else if(serv == 1)
	{
		delta = abs(newpos - now->servo1);
	}

	travel_time = delta * MOV_TIME;
	return travel_time;
}

void move_servo(int serv, position newpos)
{
	if(serv == 1)
	{
		htim2.Instance->CCR1 = newpos;
	}
	else if(serv == 2)
	{
		htim2.Instance->CCR2 = newpos;
	}
}
