/*
 * servo.h
 *
 *  Created on: Oct 28, 2022
 *      Author: Ty Freeman
 */

#ifndef INC_SERVO_H_
#define INC_SERVO_H_

TaskHandle_t npc;
TaskHandle_t player;


void NPC_Task(void * pvParameters);
void Player_Task(void * pvParameters);

#endif /* INC_SERVO_H_ */
