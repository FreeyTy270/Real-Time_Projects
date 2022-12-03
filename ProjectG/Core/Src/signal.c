/*
 * numGen.c
 *
 *  Created on: Nov 9, 2022
 *      Author: Ty Freeman
 */

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "stm32l4xx_hal.h"
#include "math.h"

#define PI 3.14159265359

extern uint16_t RRM[];
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim2;

/*Single function call for generating the ROM and changing the timer values. Called by manager task*/
void mkSig(sig_t *currSig)
{
	ROM_Gen(currSig);
	tim_adj(currSig->channel, currSig->freq, currSig->width);
}

/*Generates ROM values for signal based on signal type and max/min values*/
void ROM_Gen(sig_t *currSig)
{
	double amp = currSig->max - currSig->min; //Calc the amplitude of desired signal
	double amp_dig = (amp*4096/3.3); //Convert to digital value
	double offset = currSig->min/3.3*4096; //Include digital value of lowest voltage as offset

	/*Here is where the ROM gets created. First looks at signal type and calculates values accordingly*/
	switch(currSig->type)
	{
		case RECT:
		{
			for(int i = 0; i < currSig->width; i++)
			{
				if(i < currSig->width/2)
				{
					RRM[i] = amp_dig; //For half of period the signal is high
				}
				else
				{
					RRM[i] = 0; //For other half it is whatever minV is requested to be
				}

				RRM[i] += offset; //Add offset to calculated value

			}
			break;
		}
		case SIN:
		{
			for(int i = 0; i <= currSig->width/4; i++)
			{
				RRM[i] = (amp_dig/2)*(sin(i*2*PI/currSig->width) + 1) + offset; //Calc quarter wavelength of sine wave
				RRM[100 - i] = RRM[i]; //Copy quarter wave found above in reverse order
				RRM[100 + i] = (amp_dig/2)*(sin((100 + i)*2*PI/currSig->width) + 1) + offset; //Calc negative quarter of sine wave
				RRM[currSig->width - 1 - i] = RRM[100 + i]; //Copy this new quarter wave
			}
			break;
		}
		case TRI:
		{
			double step = amp_dig/currSig->width; //step size to reach the max amplitude in the correct number of samples

			for(int i = 0; i < currSig->width/2; i++)
			{
				RRM[i] = 2*step*i + offset; //Build first half of triangle wave
				RRM[currSig->width-1-i] = RRM[i]; //Copy to second half of signal for complete triangle
			}
			break;
		}
	}
}

/*Changes designated timer value based on the desired frequency*/
void tim_adj(_Bool ch, double freq, int width)
{
	TIM_TypeDef *Timer = NULL;

	Timer = ch ? TIM4 : TIM2; //Choose correct timer
	uint32_t oldARR = Timer->ARR; //Save previous ARR value for triggering update event

	uint32_t newARR = (TIM/(freq * width)) - 1; //Calculate new ARR value
	Timer->ARR = newARR; //Set new ARR value in buffer
	Timer->CNT = oldARR; //Trigger update event to set new timer value

}
