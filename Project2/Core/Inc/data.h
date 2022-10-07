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

enum events
{
	stop,
	cont,
	right,
	left,
	start,
	swap
};

opcode_t read_recipe(const unsigned char *recipe, int index);
int chk_loop(int repeat);
int run_inst(int dev, int position, opcode_t com);



#endif /* INC_DATA_H_ */
