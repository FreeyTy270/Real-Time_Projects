/*
 * low_level.c
 *
 *  Created on: Oct 7, 2022
 *      Author: Ty Freeman
 */

#include "low_level.h"
#include "stm32l4xx.h"

#define MOV_TIME 75
extern TIM_HandleTypeDef htim3;

static const int position[] = {25, 36, 52, 68, 84, 100};

opcode_t read_recipe(const uint8_t *recipe, int index)
{
	opcode_t command;

	command.operation = recipe[index] & 0xE0;
	command.data = recipe[index] & 0x1F;

	return command;

}

int chk_loop(int repeat)
{
	static _Bool first_flg = 1;
	static int nrepeat = 0;

	if(first_flg)
	{
		first_flg = 0;
		nrepeat = repeat;
	}
	else
	{
		nrepeat --;
	}

	return nrepeat;
}

int run_inst(int dev, int position, opcode_t com)
{
	int delay = 0;
	int dist = 0;

	switch(com.operation)
	{
	case MOV:
		dist = ((com.data - position) > 0) ? (com.data - position) : (position - com.data);
		delay = get_mov_delay(dist);
		move_servo(dev, com.data);
		break;
	case WAIT:
		delay = com.data * 100;
		break;
	case LOOP:
		delay = 32;
		break;
	case END_LOOP:
		delay = 33;
		break;
	case END_RECIPE:
		delay = 34;
		break;
	}

	return delay;
}

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
