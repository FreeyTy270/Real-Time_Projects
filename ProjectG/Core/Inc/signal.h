/*
 * numGen.h
 *
 *  Created on: Nov 9, 2022
 *      Author: Ty Freeman
 */

#ifndef INC_SIGNAL_H_
#define INC_SIGNAL_H_

#define TIM 8000000

#include "globals.h"

void mkSig(sig_t *currSig);
void ROM_Gen(sig_t *currSig);
void tim_adj(_Bool ch, double freq, int width);


#endif /* INC_SIGNAL_H_ */
