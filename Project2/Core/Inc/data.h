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

#define MOV  001
#define LOOP 100
#define WAIT 010
#define END  000

unsigned char recipe1[] = {MOV + 3, MOV | 5, RECIPE_END};
unsigned char recipe2[] = {MOV | 5, MOV | 2, RECIPE_END};

unsigned char *recipes[] = {recipe1, recipe2, NULL};

enum devices
{
	servo1,
	servo2
};

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
