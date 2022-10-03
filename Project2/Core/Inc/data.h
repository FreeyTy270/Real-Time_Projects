/*
 * data.h
 *
 *  Created on: Sep 28, 2022
 *      Author: Ty Freeman
 *
 *      Define and create all global data variables, structures, and types
 */

#ifndef INC_DATA_H_
#define INC_DATA_H_

#include <stdint.h>

#define MOV  	  	0x10
#define LOOP 	  	0x80
#define WAIT 	  	0x20
#define END_LOOP  	0x30
#define END_RECIPE	0x00

uint8_t recipe1[] = {MOV + 3, MOV | 5, END_RECIPE};
uint8_t recipe2[] = {MOV | 5, MOV | 2, END_RECIPE};

uint8_t *recipes[] = {recipe1, recipe2};

typedef struct opcode
{
	int operation;
	int data;
}opcode_t;

enum status
{
	status_running,
	status_paused,
	status_command_error,
	status_nested_error
};

enum events
{
	cont,
	right,
	left,
	stop,
	recipe_end
};
#endif /* INC_DATA_H_ */
