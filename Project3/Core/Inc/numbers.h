/*
 * time_calc.h
 *
 *  Created on: Oct 19, 2022
 *      Author: Ty Freeman
 */

#ifndef INC_NUMBERS_H_
#define INC_NUMBERS_H_
#include <inttypes.h>

#define START 9
#define END 4

enum reasons{
	mngr,
	process,
	off
};

int num_gen(int reason);
void format_time(int time_since, int *hr, int *min);
void shiftOut(uint8_t val);
void WriteNumberToSegment(int Segment, int Value);
void dig_ret(int val, int *digBuf);


#endif /* INC_NUMBERS_H_ */
