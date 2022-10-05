/*
 * servo.c
 *
 *  Created on: Sep 28, 2022
 *      Author: Ty Freeman
 */


#include "servo.h"
#include "stm32l4xx.h"

#define MOV_TIME 75

extern TIM_HandleTypeDef htim3;

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
		htim3.Instance->CCR1 = position[newpos];
	}
	else if(serv == 2)
	{
		htim3.Instance->CCR2 = position[newpos];
	}
}
