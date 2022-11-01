/*
 * servo.h
 *
 *  Created on: Oct 28, 2022
 *      Author: Ty Freeman
 */

#ifndef INC_SERVO_H_
#define INC_SERVO_H_

extern int npc_range[];
extern int player_range[];

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
	int *position;
}servo_t;

void servo_init(void);
void NPC_Task(void * pvParameters);
void Player_Task(void * pvParameters);

#endif /* INC_SERVO_H_ */
