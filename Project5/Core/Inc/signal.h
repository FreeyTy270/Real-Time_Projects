/*
 * numGen.h
 *
 *  Created on: Nov 9, 2022
 *      Author: Ty Freeman
 */

#ifndef INC_SIGNAL_H_
#define INC_SIGNAL_H_

#include <stdint.h>

#define Fs 200
#define TIM 8000000


typedef enum sigtype {
	RECT,
	TRI,
	SIN,
	ARB,
}sigType_t;

typedef struct signal {
	_Bool channel;
	sigType_t type;
	double freq;
	float amp;
	int offset;
	int noise;
	uint32_t *ROM;
}sig_t;

void mkSig(sig_t *currSig);
void ROM_Gen(sig_t *currSig);
void tim_adj(double freq);


#endif /* INC_SIGNAL_H_ */
