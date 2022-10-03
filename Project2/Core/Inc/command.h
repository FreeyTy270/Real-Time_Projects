/*
 * command.h
 *
 *  Created on: Sep 26, 2022
 *      Author: Ty Freeman
 */

#ifndef INC_COMMAND_H_
#define INC_COMMAND_H_

opcode_t read_recipe(const unsigned char *recipe, int index);
int chk_loop(int repeat);
int run_inst(int dev, int position, opcode_t com);

#endif /* INC_COMMAND_H_ */
