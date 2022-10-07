/*
 * ind_level.h
 *
 *  Created on: Oct 7, 2022
 *      Author: Ty Freeman
 */

#include <stdint.h>

#ifndef INC_IND_LEVEL_H_
#define INC_IND_LEVEL_H_

typedef struct servo servo_t;

enum events
{
	stop,
	cont,
	right,
	left,
	start,
	swap
};

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

void override_process(servo_t *servo, uint8_t cmd);
int chk_state(servo_t *servo);
void fetch_next(servo_t *servo);
int run_servo(servo_t *servo);
void chk_delay(servo_t *servo, const int delay);
void hold(servo_t *servo);
void go(servo_t *servo1, servo_t *servo2);

#endif /* INC_IND_LEVEL_H_ */
