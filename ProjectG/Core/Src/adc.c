/*
 * adc.c
 *
 *  Created on: Nov 25, 2022
 *      Author: Ty Freeman
 */

#include "globals.h"
#include "adc.h"

#define SR 20000

_Bool cap_flg = 0;
int cap_buf[SR] = {0};

void ADC_Task(void * pvParameters)
{

	while(1)
	{

		HAL_ADC_Start_DMA(&hadc1, cap_buf, SR);

	}
}
