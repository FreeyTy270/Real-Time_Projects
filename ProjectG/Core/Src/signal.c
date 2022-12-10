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
#define FS 2500
#define TIM 80000000

extern uint16_t RRM[];
uint32_t sig1_ROM[FS] = {0};
uint32_t sig2_ROM[FS] = {0};
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim2;

/*Single function call for generating the ROM and changing the timer values. Called by manager task*/
void mkSig(sig_t *currSig)
{
	ROM_Gen(currSig);
	//tim_adj(currSig->channel, currSig->freq, currSig->width);
}

/*Generates ROM values for signal based on signal type and max/min values*/
void ROM_Gen(sig_t *currSig)
{
	double amp = currSig->max - currSig->min; //Calc the amplitude of desired signal;
	int j = 0;
	int i = 0;

	/*Here is where the ROM gets created. First looks at signal type and calculates values accordingly*/
	switch(currSig->type)
	{

		case RECT:
		{
			while(j < FS)
			{
				for(i = 0; i < currSig->width; i++)
				{
					if(i < FS/2)
					{
						currSig->ROM[j] = currSig->max; //For half of period the signal is high
					}
					else
					{
						currSig->ROM[j] = currSig->min; //For other half it is whatever minV is requested to be
					}

					j++;
				}

			}
			j = 0;
			i = 0;
			break;
		}
		case SIN:
		{
			while(j < FS)
			{
				for(int i = 0; i <= currSig->width; i++)
				{
					currSig->ROM[j] = (amp/2)*(sin(i*2*PI/FS) + 1) + currSig->min; //Calc quarter wavelength of sine wave
					j++;
				}
			}
			j = 0;
			i = 0;
			break;
		}
		case TRI:
		{
			double step = amp/FS; //step size to reach the max amplitude in the correct number of samples
			while(j < FS)
			{
				for(int i = 0; i < currSig->width; i++)
				{
					currSig->ROM[j] = 2*step*i; //Build first half of triangle wave
					currSig->ROM[FS-1-j] = currSig->ROM[j];
					j++;
				}
			}
			j = 0;
			i = 0;
			break;
		}
		case ARB:
		{
			while(j < FS)
			{
				for(i = 0; i < currSig->width; i++)
					currSig->ROM[j] = RRM[i];
			}
			j = 0;
			i = 0;
		}
	}
}

/*Changes designated timer value based on the desired frequency*/
void tim_adj(_Bool ch, double freq, int width)
{
	TIM_TypeDef *Timer = NULL;

	Timer = ch ? TIM4 : TIM2; //Choose correct timer
	uint32_t oldARR = Timer->ARR; //Save previous ARR value for triggering update event

	uint32_t newARR = (TIM/(freq * FS)) - 1; //Calculate new ARR value
	Timer->ARR = newARR; //Set new ARR value in buffer
	Timer->CNT = oldARR; //Trigger update event to set new timer value

}
