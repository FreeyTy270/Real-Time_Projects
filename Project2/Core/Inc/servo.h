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

int get_mov_delay(current_state_t now, int serv, servo_state newpos);
void move_servo(int serv, position newpos);

#endif /* INC_SERVO_H_ */
