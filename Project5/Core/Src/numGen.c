/*
 * numGen.c
 *
 *  Created on: Nov 9, 2022
 *      Author: Ty Freeman
 */

#include <stdlib.h>
#include "stm32l4xx_hal.h"
#include "numGen.h"
#include "math.h"

#define PI 3.14159265359

extern TIM_HandleTypeDef htim2;

void signal_Gen(sig_t *currSig)
{
	switch(currSig->type)
	case RECT:
	{
		calc_dig(currSig->amp);
		break;
	}
	case SIN:
	{
		for(int i = 0; i < Fs; i++)
		{
			currSig->ROM[i] = (sin(i*2*PI/Fs) + 1)*(currSig->amp/3.3)*4096;
		}
		break;
	}
	case TRI:
	{
		float step = ((currSig->amp/3.3)*4096)/Fs;

		for(int i = 0; i < Fs)
	}
}


void timr_adj(int freq)
{
	int newPSC = freq * Fs;


}

void calc_dig(int val, int *ROM)
{
	int i = 0;
	for(int i = 0; i < Fs; i++)
	{
		ROM[i] = (4096 * val) / 3.3;
	}
}
