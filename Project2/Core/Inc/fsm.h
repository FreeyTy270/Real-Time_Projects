/*
 * process.h
 *
 *  Created on: Sep 28, 2022
 *      Author: Ty Freeman
 *
 *      Checks current states and inputs from recipe or user to decide next state.
 */

#ifndef INC_FSM_H_
#define INC_FSM_H_


typedef struct current_state
{
	int servo1 = unknown;
	int servo2 = unknown;
}current_state_t;

typedef enum
{
	pos1,
	pos2,
	pos3,
	pos4,
	pos5,
	unknown,
	moving,
	recipe_ended
}servo_state;

/* Fetch Functions */
current_state_t get_state();
int get_instruction(unsigned char *recipe);



/* Action Functions */
void execute(); // The top of the program
void startup(); // Initializes all data structures and prepares device
void transition_state(); // Points next instruction to the correct state




#endif /* INC_FSM_H_ */
