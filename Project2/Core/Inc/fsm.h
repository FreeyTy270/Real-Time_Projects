/*
 * fsm.h
 *
 *  Created on: Sep 28, 2022
 *      Author: Ty Freeman
 *
 *      Checks current states and inputs from recipe or user to decide next state.
 */

#ifndef INC_FSM_H_
#define INC_FSM_H_

typedef struct system_state system_state_t;

/* Helper Functions */
int max(int num1, int num2);

/* Action Functions */
void execute(_Bool *flg); // The top of the program
void startup(system_state_t *system); // Initializes all data structures and prepares device
void sys_fetch_next(system_state_t *system, int flg); // Points next instruction to the correct state
int chk_states(system_state_t *system);
int run_next(system_state_t *system);


#endif /* INC_FSM_H_ */
