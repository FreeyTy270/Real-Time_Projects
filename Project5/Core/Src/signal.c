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
uint32_t ekg[] = {
	1690, 1680, 1680, 1669, 1648, 1648, 1648, 1680, 1680, 1690, 1680, 1680, 1680, 1680, 1680, 1658,
	1690, 1690, 1712, 1690, 1690, 1680, 1669, 1669, 1680, 1690, 1690, 1680, 1669, 1669, 1669, 1680,
	1680, 1680, 1669, 1669, 1680, 1690, 1690, 1680, 1690, 1690, 1680, 1690, 1690, 1712, 1680, 1680,
	1658, 1648, 1648, 1648, 1669, 1669, 1680, 1690, 1690, 1701, 1680, 1680, 1669, 1680, 1680, 1680,
	1701, 1701, 1701, 1690, 1690, 1701, 1712, 1712, 1722, 1712, 1712, 1690, 1669, 1669, 1680, 1690,
	1690, 1690, 1733, 1733, 1765, 1776, 1861, 1882, 1936, 1936, 1968, 1989, 1989, 2032, 2053, 2053,
	2085, 2149, 2069, 2080, 2058, 2058, 1930, 1930, 1845, 1824, 1792, 1872, 1840, 1754, 1754, 1722,
	1680, 1680, 1680, 1637, 1637, 1637, 1637, 1637, 1626, 1648, 1648, 1637, 1605, 1605, 1616, 1680,
	1680, 1765, 1776, 1861, 2042, 2106, 2021, 1776, 2480, 2400, 2176, 1632, 1637, 1360, 933,  928,
	1962, 1962, 2042, 2149, 3141, 3141, 2320, 1200, 1200, 1392, 1669, 1669, 1658, 1701, 1701, 1701,
	1701, 1701, 1722, 1690, 1690, 1690, 1680, 1680, 1690, 1690, 1690, 1669, 1669, 1669, 1701, 1733,
	1733, 1754, 1744, 1744, 1733, 1733, 1733, 1722, 1765, 1765, 1765, 1733, 1733, 1733, 1722, 1722,
	1701, 1690, 1690, 1701, 1690, 1690, 1701, 1701, 1701, 1701, 1722, 1722, 1712, 1722, 1722, 1733,
	1733, 1733, 1733, 1712, 1712, 1712, 1733, 1733, 1733, 1733, 1733, 1733, 1744, 1744, 1744, 1744,
	1744, 1744, 1733, 1733, 1722, 1722, 1722, 1722, 1722, 1722, 1733, 1722, 1722, 1722, 1722, 1722,
	1701, 1669, 1669, 1680, 1690, 1690, 1690, 1701, 1701, 1712, 1712, 1712, 1690, 1669, 1669, 1680,
};

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern RNG_HandleTypeDef hrng;

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

	uint32_t rndNum = 0;
	uint16_t bitMask = 0;
	uint16_t noiseStp = 1;

	/*Noise calculation based on number of noisy bits requested*/
	if(currSig->noise > 0)
	{
		HAL_RNG_GenerateRandomNumber(&hrng, &rndNum);

		rndNum &= 0xFFF;
		noiseStp = 0xF;

		int tmpMask = 0;

		for(int i = 1; i < currSig->noise; i++)
		{
			tmpMask = 1 << (currSig->noise - i);
			bitMask |= tmpMask;
		}

		rndNum &= bitMask;
	}

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

				if(i % noiseStp == 0) //Distance between noise affected samples
				{
					currSig->ROM[i] += rndNum; // Add noise to random sample
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
				if(i % noiseStp == 0)
				{
					currSig->ROM[i] += rndNum; //Add noise
				}
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
				if(i % noiseStp == 0)
				{
					currSig->ROM[i] += rndNum; //Add noise
				}
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
				if(i % noiseStp == 0)
				{
					currSig->ROM[i] += rndNum; //Add noise
				}
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
