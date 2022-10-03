/*
 * data.c
 *
 *  Created on: Oct 3, 2022
 *      Author: Ty Freeman
 */

#include "data.h"
#include "servo.h"
#include "stm32l4xx.h"

uint8_t recipe1[] = {MOV + 3, MOV | 5, END_RECIPE};
uint8_t recipe2[] = {MOV | 5, MOV | 2, END_RECIPE};

uint8_t *recipes[] = {recipe1, recipe2};

opcode_t read_recipe(const uint8_t *recipe, int index)
{
	opcode_t command;

	command.operation = recipe[index] & 0x18;
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
		delay = com.data;
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
	default:
		delay = 35;
	}

	return delay;
}
