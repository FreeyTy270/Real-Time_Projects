/*
 * dac.c
 *
 *  Created on: Nov 25, 2022
 *      Author: Ty Freeman
 */

#include "globals.h"
#include "dac.h"


void DAC_Task(void * pvParameters)
{
	TickType_t Period = pdMS_TO_TICKS(20);
	TickType_t lastWake = 0;
	sig_t sigReq;

	sig_t signal_1;
	sig_t signal_2;


	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim4);

	HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, sig1_ROM, Fs, DAC_ALIGN_12B_R);
	HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_2, sig2_ROM, Fs, DAC_ALIGN_12B_R);

	while(1)
	{
		if(cmd_flg)
		{
			cmd_flg = 0;
			if(xQueueReceive(msgQ, &sigReq, Period) != pdTRUE)
			{
				printf("Could not retrieve signal request from queue\n\r");
			}

			if(sigReq.channel)
			{
				signal_2 = sigReq;
				signal_2.ROM = sig2_ROM;
				HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_2);
				mkSig(&signal_2);
				HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_2, sig2_ROM, Fs, DAC_ALIGN_12B_R);
			}
			else
			{
				signal_1 = sigReq;
				signal_1.ROM = sig1_ROM;
				HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
				mkSig(&signal_1);
				HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, sig1_ROM, Fs, DAC_ALIGN_12B_R);
			}

		}


		lastWake = xTaskGetTickCount();
		vTaskDelayUntil(&lastWake, Period);

	}
}
