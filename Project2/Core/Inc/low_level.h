/*
 * low_level.h
 *
 *  Created on: Oct 7, 2022
 *      Author: Ty Freeman
 */

#include "stdint.h"
#ifndef INC_LOW_LEVEL_H_
#define INC_LOW_LEVEL_H_

#define MOV  	  	0x20
#define WAIT 	  	0x40
#define LOOP 	  	0x80
#define END_LOOP  	0xA0
#define END_RECIPE	0x00

typedef struct opcode
{
	int operation;
	int data;
}opcode_t;

opcode_t read_recipe(const uint8_t *recipe, int index);
int chk_loop(int repeat);
int run_inst(int dev, int position, opcode_t com);

int get_mov_delay(int distance);
void move_servo(int serv, int newpos);

#endif /* INC_LOW_LEVEL_H_ */
