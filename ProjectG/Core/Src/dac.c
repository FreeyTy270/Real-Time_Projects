/*
 * dac.c
 *
 *  Created on: Nov 30, 2022
 *      Author: Ty Freeman
 */

#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stm32l4xx_hal.h"
#include "signal.h"
#include "dac.h"


extern DAC_HandleTypeDef hdac1;
extern DMA_HandleTypeDef hdma_dac_ch1;
extern DMA_HandleTypeDef hdma_dac_ch2;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;

extern uint16_t cap_buf[];

void mng_Task(void * pvParameters)
{
	TickType_t Period = pdMS_TO_TICKS(20);
	TickType_t lastWake = 0;
	sig_t sigReq;

	sig_t signal_1;
	sig_t signal_2;



	while(1)
	{
		if(cap_flg)
		{
			HAL_TIM_Base_Start(&htim2);
			HAL_TIM_Base_Start(&htim4);

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
