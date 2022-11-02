/*
 * servo.h
 *
 *  Created on: Oct 28, 2022
 *      Author: Ty Freeman
 */

#ifndef INC_SERVO_H_
#define INC_SERVO_H_


enum servo_state{
	calibratingL,
	calibratingR,
	gaming,
	stopped
};

enum servo_pos{
	pos0,
	pos1,
	pos2,
	pos3,
	pos4,
	pos5
};

typedef struct servo{
	int currPos;
	int currState;
	_Bool cal;
	int round_cnt;
	int position[6];
}servo_t;

typedef struct flags{
	_Bool start; // Player Started Game Flag
	_Bool prog_btn_flg;
	_Bool mv_left;
	_Bool mv_right;
}flags_t;

void servo_init(void);
void calibration_Task(void * pvParameters);
void NPC_Task(void * pvParameters);
void Player_Task(void * pvParameters);

#endif /* INC_SERVO_H_ */
