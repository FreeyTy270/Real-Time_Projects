/*
 * adc.c
 *
 *  Created on: Nov 21, 2022
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

double todig = 4096/3.3;
double toreal = 3.3/4096;


void adc_Task(void * pvParameters)
{
	TickType_t lastwake = 0;

	HAL_DAC_Stop_DMA(&hdac1, 1);
	HAL_DAC_Stop_DMA(&hdac1, 2);

	HAL_TIM_Base_Start_IT(&htim6);
	sum = 0;

	while(1)
	{
		if(full)
		{
			HAL_TIM_Base_Stop_IT(&htim6);
			full = 0;
			calc_sig();
			adc_done = 1;
			vTaskDelete(NULL);
		}
		lastwake = xTaskGetTickCount();
		vTaskDelayUntil(&lastwake, pdMS_TO_TICKS(20));
	}
}

void calc_sig(void)
{
	double sig = 0;
	float wiggle = 0.05;
	float prev = 0;
	float post = 0;
	int VPP_dig = newSig.max - newSig.min;
	float VPP = VPP_dig * toreal;
	int dist = mindx;


	sig = sigma_calc();

	if(VPP >= 2*sig - wiggle && VPP <= 2*sig + wiggle)
		newSig.type = RECT;
	else if(VPP >= sqrt(12)*sig - wiggle && VPP <= sqrt(12)*sig + wiggle)
		newSig.type = TRI;
	else if(VPP >= sqrt(8)*sig - wiggle && VPP <= sqrt(8)*sig + wiggle)
		newSig.type = SIN;
	else
		newSig.type = ARB;

	if(newSig.type == RECT)
	{
		do
		{
			dist++;
			prev = (RRM[dist]*toreal) - (RRM[dist - 1]*toreal);
		}while(prev <= VPP - 10);
		newSig.freq = 1/((dist - mindx) * 2 * 0.0001);
	}
	else if(newSig.type == ARB)
	{
		newSig.freq = -1;
	}
	else
	{
		do
		{
			dist++;
			prev = (RRM[dist - 1]*toreal) - (RRM[dist]*toreal);
			post = (RRM[dist + 1]*toreal) - (RRM[dist]*toreal);
		}while(post > 0 && prev < 0);
		newSig.freq = 1/((dist - mindx) * 2 * 0.0001);
	}

}


double sigma_calc(void)
{
	double mean = 0;
	double total = 0;
	double x = 0;

	mean = (sum / SR) * toreal;

	for(int i = 0; i < SR; i++)
	{
		x = (RRM[i] * toreal) - mean;
		total += (x * x);
	}

	return sqrt(total / (SR - 1));
}
