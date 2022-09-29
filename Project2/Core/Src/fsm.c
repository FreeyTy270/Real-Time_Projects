/*
 * fsm.c
 *
 *  Created on: Sep 28, 2022
 *      Author: Ty Freeman
 */

#include "fsm.h"
#include "data.h"


void execute()
{
	startup();

}

void startup()
{

}

current_state_t get_state()
{
	current_state_t now;
	return now;
}

void set_state(current_state_t new, int serv, int new_state)
{
	new.serv = new_state;
}
