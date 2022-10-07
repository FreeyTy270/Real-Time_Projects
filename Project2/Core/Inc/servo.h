/*
 * servo.h
 *
 *  Created on: Sep 28, 2022
 *      Author: Ty Freeman
 *
 *      Servo control logic
 */

#ifndef INC_SERVO_H_
#define INC_SERVO_H_

static const int position[] = {25, 36, 52, 68, 84, 100};


int get_mov_delay(int distance);
void move_servo(int serv, int newpos);

#endif /* INC_SERVO_H_ */
