/*
 * numGen.c
 *
 *  Created on: Nov 9, 2022
 *      Author: Ty Freeman
 */

#include <signal.h>
#include <stdlib.h>
#include "stm32l4xx_hal.h"
#include "math.h"

#define PI 3.14159265359

uint32_t sig1_ROM[Fs] = {0};
uint32_t sig2_ROM[Fs] = {0};

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;

/*Single function call for generating the ROM and changing the timer values. Called by manager task*/

void mkSig(sig_t *currSig)
{
	ROM_Gen(currSig);
	tim_adj(currSig->channel, currSig->freq);
}

/*Generates ROM values for signal based on signal type and max/min values*/
void ROM_Gen(sig_t *currSig)
{
	double amp = currSig->maxV - currSig->minV; //Calc the amplitude of desired signal
	double amp_dig = (amp*4096/3.3); //Convert to digital value
	double offset = currSig->minV/3.3*4096; //Include digital value of lowest voltage as offset

	/*Here is where the ROM gets created. First looks at signal type and calculates values accordingly*/
	switch(currSig->type)
	{
		case RECT:
		{
			for(int i = 0; i < Fs; i++)
			{
				if(i < Fs/2)
				{
					currSig->ROM[i] = amp_dig; //For half of period the signal is high
				}
				else
				{
					currSig->ROM[i] = 0; //For other half it is whatever minV is requested to be
				}

				currSig->ROM[i] += offset; //Add offset to calculated value

			}
			break;
		}
		case SIN:
		{
			for(int i = 0; i <= Fs/4; i++)
			{
				currSig->ROM[i] = (amp_dig/2)*(sin(i*2*PI/Fs) + 1) + offset; //Calc quarter wavelength of sine wave
				currSig->ROM[100 - i] = currSig->ROM[i]; //Copy quarter wave found above in reverse order
				currSig->ROM[100 + i] = (amp_dig/2)*(sin((100 + i)*2*PI/Fs) + 1) + offset; //Calc negative quarter of sine wave
				currSig->ROM[Fs - 1 - i] = currSig->ROM[100 + i]; //Copy this new quarter wave
			}
			break;
		}
		case TRI:
		{
			double step = amp_dig/Fs; //step size to reach the max amplitude in the correct number of samples

			for(int i = 0; i < Fs/2; i++)
			{
				currSig->ROM[i] = 2*step*i + offset; //Build first half of triangle wave
				currSig->ROM[Fs-1-i] = currSig->ROM[i]; //Copy to second half of signal for complete triangle
			}
			break;
		}
		case ARB:
		{
			int arb_amp = 3141 - 928; //Max ekg value - min value to find range
			double scale = amp_dig/arb_amp; // calc scale value for desired amplitude


			for(int i = 0; i < Fs; i++)
			{
				currSig->ROM[i] = (uint32_t) (scale*(ekg[i] - 928) + offset); // scale ekg value and set within bounds
			}
		}
	}
}

/*Changes designated timer value based on the desired frequency*/
void tim_adj(_Bool ch, double freq)
{
	TIM_TypeDef *Timer = NULL;

	Timer = ch ? TIM4 : TIM2; //Choose correct timer
	uint32_t oldARR = Timer->ARR; //Save previous ARR value for triggering update event

	uint32_t newARR = (TIM/(freq * Fs)) - 1; //Calculate new ARR value
	Timer->ARR = newARR; //Set new ARR value in buffer
	Timer->CNT = oldARR; //Trigger update event to set new timer value

}
