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
	recipe_ended,
	serv_pos1,
	serv_pos2,
	serv_pos3,
	serv_pos4,
	serv_pos5,
	serv_moving,
	serv_unknown,
};

enum status
{
	status_running,
	status_paused,
	status_cmd_error,
	status_loop_error
};

/* Helper Functions */
int chk_state(servo_t *servo);
void fetch_next(servo_t *servo);
int max(int num1, int num2);
void chk_delay(servo_t *servo, const int delay);

/* Action Functions */
void execute(_Bool *flg); // The top of the program
void override_process(servo_t *servo, uint8_t cmd);
void startup(system_state_t *system); // Initializes all data structures and prepares device
void fetch_next_sys(system_state_t *system, int flg); // Points next instruction to the correct state
int chk_states(system_state_t *system);
int run_next(system_state_t *system);
void hold(servo_t *servo);


#endif /* INC_FSM_H_ */
