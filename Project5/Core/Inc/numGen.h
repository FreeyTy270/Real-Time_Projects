/*
 * numGen.h
 *
 *  Created on: Nov 9, 2022
 *      Author: Ty Freeman
 */

#ifndef INC_NUMGEN_H_
#define INC_NUMGEN_H_

#define Fs 200

typedef enum sigtype {
	RECT,
	TRI,
	SIN,
	ARB,
}sigType_t;

typedef struct signal {
	_Bool channel;
	sigType_t type;
	int freq;
	float amp;
	int offset;
	int noise;
	uint32_t *ROM;
}sig_t;


void signal_Gen(sig_t *currSig);


#endif /* INC_NUMGEN_H_ */
