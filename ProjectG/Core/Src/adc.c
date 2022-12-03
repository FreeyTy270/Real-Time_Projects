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

#include "globals.h"
#include "adc.h"

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim6;
extern sig_t newSig;

void adc_Task(void * pvParameters)
{
	TickType_t lastwake = 0;

	printf("Stopping DAC for conversions...\n\r");
	HAL_DAC_Stop_DMA(&hadc1, 1);
	HAL_DAC_Stop_DMA(&hadc1, 2);
	HAL_TIM_Base_Start_IT(&htim6);

	while(1)
	{
		if(full)
		{
			HAL_TIM_Base_Stop_IT(&htim6);
			printf("...Doing Math...\n\r");
			adc_done = 1;
			full = 0;
			vTaskDelete(NULL);
		}
		lastwake = xTaskGetTickCount();
		vTaskDelayUntil(&lastwake, pdMS_TO_TICKS(20));
	}
}
