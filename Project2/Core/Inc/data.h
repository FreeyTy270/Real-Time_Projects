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

#define MOV  	  	0x10
#define LOOP 	  	0x80
#define WAIT 	  	0x20
#define END_LOOP  	0x30
#define END_RECIPE	0x00

unsigned char recipe1[] = {MOV + 3, MOV | 5, RECIPE_END};
unsigned char recipe2[] = {MOV | 5, MOV | 2, RECIPE_END};

unsigned char *recipes[] = {recipe1, recipe2, NULL};

typedef struct servo
{
	int dev;
	servo_state position;
	unsigned char *recipe;
	int recipe_index;
}servo_t;

typedef struct system_state
{
	servo_t *servo1;
	servo_t *servo2;

}system_state_t;

typedef struct opcode
{
	int operation : 3;
	int data : 5;
}opcode_t;

enum status
{
	status_running,
	status_paused,
	status_command_error,
	status_nested_error
}status;

enum events
{
	cont,
	right,
	left,
	stop,
	recipe_end
}event;
#endif /* INC_DATA_H_ */
