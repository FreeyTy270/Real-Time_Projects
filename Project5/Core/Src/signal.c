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

void mkSig(sig_t *currSig)
{
	ROM_Gen(currSig);
	tim_adj(currSig->channel, currSig->freq);
}

void ROM_Gen(sig_t *currSig)
{
	double amp_dig = (currSig->amp*4096/3.3);

	switch(currSig->type)
	{
		case RECT:
		{
			for(int i = 0; i < Fs; i++)
			{
				if(i < Fs/2)
				{
					currSig->ROM[i] = amp_dig;
				}
				else
				{
					currSig->ROM[i] = 0;
				}
			}
			break;
		}
		case SIN:
		{
			for(int i = 0; i <= Fs/4; i++)
			{
				currSig->ROM[i] = (amp_dig/2)*(sin(i*2*PI/Fs) + currSig->offset);
				currSig->ROM[100 - i] = currSig->ROM[i];
				currSig->ROM[100 + i] = (amp_dig/2)*(sin((100 + i)*2*PI/Fs) + currSig->offset);
				currSig->ROM[Fs - 1 - i] = currSig->ROM[100 + i];
			}
			break;
		}
		case TRI:
		{
			double step = amp_dig/Fs;

			for(int i = 0; i < Fs/2; i++)
			{
				currSig->ROM[i] = 2*step*i;
				currSig->ROM[Fs-1-i] = currSig->ROM[i];
			}
			break;
		}
		case ARB:
		{
			for(int i = 0; i <= Fs; i++)
			{
				currSig->ROM[i] = (ekg[i] / amp_dig) * ekg[i];
			}
		}
	}
}


void tim_adj(_Bool ch, double freq)
{
	TIM_TypeDef *Timer = NULL;

	Timer = ch ? TIM4 : TIM2;
	uint32_t oldARR = Timer->ARR;

	uint32_t newARR = (TIM/(freq * Fs)) - 1;
	Timer->ARR = newARR;
	Timer->CNT = oldARR;

}
