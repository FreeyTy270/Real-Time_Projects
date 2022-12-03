/*
 * dac.c
 *
 *  Created on: Nov 30, 2022
 *      Author: Ty Freeman
 */

#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "globals.h"
#include "signal.h"
#include "dac.h"

extern uint16_t RRM[];
extern QueueHandle_t mbx;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern DAC_HandleTypeDef hdac1;
extern DMA_HandleTypeDef hdma_dac_ch1;
extern DMA_HandleTypeDef hdma_dac_ch2;

void dac_Task(void * pvParameters)
{
	TickType_t Period = pdMS_TO_TICKS(20);
	//TickType_t wait = pdMS_TO_TICKS(2300);
	TickType_t lastWake = 0;
	sig_t sigReq;

	sig_t signal_1;
	sig_t signal_2;

	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim4);

	HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (unsigned long *) RRM, SR, DAC_ALIGN_12B_R);
	HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_2, (unsigned long *) RRM, SR, DAC_ALIGN_12B_R);

	while(1)
	{

		if(xQueueReceive(mbx, &sigReq, 1) == pdTRUE)
		{

			if(sigReq.channel)
			{
				signal_2 = sigReq;
				HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_2);
				mkSig(&signal_2);
				HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (unsigned long *) RRM, SR, DAC_ALIGN_12B_R);
			}
			else
			{
				signal_1 = sigReq;
				HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
				mkSig(&signal_1);
				HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (unsigned long *) RRM, SR, DAC_ALIGN_12B_R);
			}

		}


		lastWake = xTaskGetTickCount();
		vTaskDelayUntil(&lastWake, Period);

	}
}
