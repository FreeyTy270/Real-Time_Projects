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
#include "stm32l4xx_hal.h"
#include "adc.h"

#define SR 20000

_Bool cap_flg = 0;
_Bool dir = 0;
int min = 4095;
uint16_t cap_buf[SR] = {0};

extern TIM_HandleTypeDef htim6;
extern TaskHandle_t adc;


void adc_Task(void * pvParameters)
{
	TickType_t lastwake = 0;
	HAL_TIM_Base_Start_IT(&htim6);

	while(1)
	{
		if(cap_flg)
		{
			cap_flg = 0;
			for(int i = 0; i < SR; i++)
			{
				cap_buf[i] = 0;
			}
		}
		lastwake = xTaskGetTickCount();
		vTaskDelayUntil(&lastwake, pdMS_TO_TICKS(20));
	}
}
