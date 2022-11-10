/*
 * numGen.h
 *
 *  Created on: Nov 9, 2022
 *      Author: Ty Freeman
 */

#ifndef INC_NUMGEN_H_
#define INC_NUMGEN_H_

int *sig_ROM = NULL;

typedef enum sigtype {
	RECT,
	TRI,
	SIN,
	ARB,
}sig_t;

typedef struct sig_characteristics {
	_Bool channel;
	sig_t type;
	int freq;
	int amp;
	int offset;
	int noise;
}sigCh_t;


void signal_Gen(sigCh_t *currSig);


#endif /* INC_NUMGEN_H_ */
