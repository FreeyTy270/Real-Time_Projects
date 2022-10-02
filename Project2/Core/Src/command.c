/*
 * command.c
 *
 *  Created on: Sep 26, 2022
 *      Author: Ty Freeman
 */

#include "data.h"
#include "command.h"
#include "stm32l4xx.h"

opcode_t read_recipe(const unsigned char *recipe, int index)
{
	opcode_t command;

	command.operation = recipe[index] & 0xE0;
	command.data = recipe[index] & 0x1F;

	return command;

}

int chk_loop(servo_t *servo)
{
	if(servo->loop_flg)
	{
		servo->new_com == servo->old_com;
	}
}

int run_inst(servo_t *servo)
{
	switch(servo->new_com.operation)
	{
	case MOV:
		move_servo(servo->dev, servo->new_com.data);
		break;
	case LOOP:
		servo->recipe_index -= 1;
		return servo->new_com.data;
	case WAIT:


	}
}
