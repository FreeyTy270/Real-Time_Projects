/*
 * data.h
 *
 *  Created on: Sep 28, 2022
 *      Author: Ty Freeman
 *
 *      Declare and create all global data variables, structures, and types. Also holds declarations for
 *      functions holding command control logic.
 */

#ifndef INC_DATA_H_
#define INC_DATA_H_

#include <stdint.h>

#define MOV  	  	0x10
#define LOOP 	  	0x80
#define WAIT 	  	0x20
#define END_LOOP  	0x30
#define END_RECIPE	0x00

typedef struct opcode
{
	int operation;
	int data;
}opcode_t;

enum events
{
	cont,
	right,
	left,
	stop,
	start,
	swap
};

opcode_t read_recipe(const unsigned char *recipe, int index);
int chk_loop(int repeat);
int run_inst(int dev, int position, opcode_t com);



#endif /* INC_DATA_H_ */
