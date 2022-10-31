/*
 * servo.c
 *
 *  Created on: Oct 28, 2022
 *      Author: Ty Freeman
 */

#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "string.h"
#include "math.h"
#include "servo.h"
#include "stm32l4xx_hal.h"


extern RNG_HandleTypeDef hrng;
extern TIM_HandleTypeDef htim3;

static const int position[] = {50, 60, 70, 80, 90};

void NPC_Task(void * pvParameters)
{
	uint32_t rndNum = 0;
	int pos = 0;

	TickType_t lastwake = 0;

	TickType_t freq = pdMS_TO_TICKS(50);

	while(1)
	{
		HAL_RNG_GenerateRandomNumber(&hrng, &rndNum);

		pos = rndNum & 0x7;

		if(pos > 5)
		{
			pos = 5;
		}
		else if(pos < 0)
		{
			pos = 0;
		}

		TIM3->CCR1 = position[pos];

		printf("Moving to position %d\n\r", pos);

		lastwake = xTaskGetTickCount();
		vTaskDelayUntil(&lastwake, freq);
	}

}

void Player_Task(void * pvParameters)
{
	while(1)
	{
		vTaskDelay(500);
	}
}
