/*
 * adc.c
 *
 *  Created on: Nov 25, 2022
 *      Author: Ty Freeman
 */

#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "math.h"
#include "stm32l4xx_hal.h"

#include "globals.h"
#include "adc.h"

#define SR 20000

extern ADC_HandleTypeDef hadc1;
extern DAC_HandleTypeDef hdac1;
extern TIM_HandleTypeDef htim6;
extern uint16_t RRM[];
extern sig_t newSig;
extern int mindx;
extern int maxdx;


void adc_Task(void * pvParameters)
{
	TickType_t lastwake = 0;

	HAL_DAC_Stop_DMA(&hdac1, 1);
	HAL_DAC_Stop_DMA(&hdac1, 2);

	HAL_TIM_Base_Start_IT(&htim6);

	while(1)
	{
		if(full)
		{
			HAL_TIM_Base_Stop_IT(&htim6);
			calc_sig();
			adc_done = 1;
			full = 0;
			vTaskDelete(NULL);
		}
		lastwake = xTaskGetTickCount();
		vTaskDelayUntil(&lastwake, pdMS_TO_TICKS(20));
	}
}

void calc_sig(void)
{
	double sig = 0;
	int VPP = newSig.max - newSig.min;
	int dist = mindx;

	while(RRM[dist] != newSig.min)
	{
		dist++;
	}

	newSig.freq = 1/(dist*0.0001);

	sig = sigma_calc();

	if(VPP >= 2*sig - 5  || VPP <= 2*sig + 5)
		newSig.type = RECT;
	else if(VPP >= sqrt(12)*sig - 5 || VPP <= sqrt(12)*sig + 5)
		newSig.type = TRI;
	else if(VPP >= sqrt(8)*sig - 5 || VPP <= sqrt(8)*sig + 5)
		newSig.type = SIN;
	else
		newSig.type = ARB;
}


double sigma_calc(void)
{
	double mean = 0;
	double total = 0;

	mean = sum / SR;

	for(int i = 0; i < SR; i++)
		total += ((RRM[i] - mean) * (RRM[i] - mean));

	return sqrt(total / (SR - 1));
}
