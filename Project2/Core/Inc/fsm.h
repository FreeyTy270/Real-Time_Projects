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

typedef struct servo servo_t;
typedef struct system_state system_state_t;

enum servo_state
{
	serv_pos1,
	serv_pos2,
	serv_pos3,
	serv_pos4,
	serv_pos5,
	serv_unknown,
	serv_moving,
	recipe_ended,
};

/* Helper Functions */
void fetch_next(servo_t *servo);
void set_states(system_state_t *new, int new_state1, int new_state2);
void set_state(servo_t *serv, int new_state);
uint8_t max(uint8_t num1, uint8_t num2);
void chk_delay(servo_t *servo, const int delay);
int chk_state(servo_t *servo);

/* Action Functions */
void execute(int flg); // The top of the program
void startup(system_state_t *now); // Initializes all data structures and prepares device
void fetch_next_sys(system_state_t *system, int flg); // Points next instruction to the correct state
void run_next(system_state_t *system);
void hold(servo_t *servo);
int chk_states(system_state_t *system);




#endif /* INC_FSM_H_ */
