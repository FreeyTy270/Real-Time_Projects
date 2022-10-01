/*
 * command.c
 *
 *  Created on: Sep 26, 2022
 *      Author: Ty Freeman
 */

#include "data.h"
#include "command.h"
#include "stm32l4xx.h"


void fetch_inst(servo_t serv, opcode_t com)
{
	com = read_recipe();

	switch(com.operation)
	{
	case MOV:
		set_state(serv, serv_moving);
	case LOOP:
		set_state(serv, com.data);
	case END_LOOP:
		set_state(serv, com.data);
	case END_RECIPE:
		set_state(serv, com.data);
	}



}

opcode_t read_recipe(const unsigned char *recipe, int duration)
{
	opcode_t command;
	static int n = 0;
	/*if(n > duration)
	{
		command.operation = 0;
		command.data = 0;

		return command;
	}*/

	command.operation = recipe[n] & 0xE0;
	command.data = recipe[n] & 0x1F;

	return command;

}
