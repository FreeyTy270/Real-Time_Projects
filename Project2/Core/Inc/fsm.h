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


typedef struct system_state
{
	servo_t *servo1;
	servo_t *servo2;

}system_state_t;

typedef enum
{
	serv_pos1,
	serv_pos2,
	serv_pos3,
	serv_pos4,
	serv_pos5,
	serv_unknown,
	serv_moving,
	recipe_ended,
}servo_state;

/* Fetch Functions */
current_state_t get_state();
int get_instruction(unsigned char *recipe);



/* Action Functions */
void execute(); // The top of the program
void startup(); // Initializes all data structures and prepares device
void chk_state(); // Points next instruction to the correct state





#endif /* INC_FSM_H_ */
