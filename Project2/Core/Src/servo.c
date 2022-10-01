/*
 * servo.c
 *
 *  Created on: Sep 28, 2022
 *      Author: Ty Freeman
 */


#include "servo.h"
#include "data.h"

#define MOV_TIME 75

extern TIM_HandleTypeDef htim2;

static const position[] = {50, 63, 75, 88, 100};

int get_mov_delay(int distance)
{
	int travel_time = 0;

	travel_time = distance * MOV_TIME;
	return travel_time;
}

void move_servo(int serv, int newpos)
{
	if(serv == 1)
	{
		htim2.Instance->CCR1 = position[newpos];
	}
	else if(serv == 2)
	{
		htim2.Instance->CCR2 = position[newpos];
	}
}
