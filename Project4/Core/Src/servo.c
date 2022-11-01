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
#include "stm32l4xx_hal.h"
#include "math.h"

#include "shld_def.h"
#include "servo.h"


extern RNG_HandleTypeDef hrng;
extern TIM_HandleTypeDef htim3;
extern TaskHandle_t npc;
extern TaskHandle_t player;

servo_t servoN = {pos0, stopped, 0, 0, {42, 0, 0, 0, 0, 0}};
servo_t servoP = {pos0, stopped, 0, 0, {42, 0, 0, 0, 0, 0}};


void servo_init()
{
	TIM3->CCR1 = servoN.position[pos0];
	TIM3->CCR2 = servoP.position[pos0];

	servoN.currPos = pos0;
	servoP.currPos = pos0;
}

void NPC_Task(void * pvParameters)
{
	uint32_t rndNum = 0;
	int pos = 0;
	static _Bool prnt_flg = 0;

	TickType_t lastwake = 0;
	TickType_t stop_wait = pdMS_TO_TICKS(100);
	TickType_t debounce = pdMS_TO_TICKS(70);
	TickType_t freq = pdMS_TO_TICKS(200);

	while(1)
	{
		if(!servoN.cal && servoN.currState != stopped)
		{
			if(!prnt_flg)
			{
				printf("CAL1\n\r");
				prnt_flg = 1;
			}

			while(HAL_GPIO_ReadPin(SHLD_A1_GPIO_Port, SHLD_A1_Pin)==GPIO_PIN_RESET)
			{
				TIM3->CCR1 -= 5;
				lastwake = xTaskGetTickCount();
				vTaskDelayUntil(&lastwake, stop_wait);
			}

			while(HAL_GPIO_ReadPin(SHLD_A3_GPIO_Port, SHLD_A3_Pin)==GPIO_PIN_RESET)
			{
				TIM3->CCR1 += 5;
				lastwake = xTaskGetTickCount();
				vTaskDelayUntil(&lastwake, stop_wait);
			}

			if(servoN.currState == calibratingL)
			{
				if(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET)
				{
					lastwake = xTaskGetTickCount();
					vTaskDelayUntil(&lastwake, debounce);
					if(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET)
					{
						servoN.position[pos0] = TIM3->CCR1;
						printf("Low Measurement Captured\n\r");
						servoN.currState = calibratingR;
					}
				}
			}
			else if(servoN.currState == calibratingR)
			{
				if(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET)
				{
					lastwake = xTaskGetTickCount();
					vTaskDelayUntil(&lastwake, debounce);
					if(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET)
					{
						servoN.position[pos5] = TIM3->CCR1;
						TIM3->CCR1 = servoN.position[pos0];
						printf("High Measurement Captured\n\r");
						servoN.currState = stopped;
						servoP.currState = calibratingL;
						servoN.cal = 1;
						vTaskSuspend(npc);
					}
				}
			}
		}

		else if(start && go)
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

			TIM3->CCR1 = servoN.position[pos];

			lastwake = xTaskGetTickCount();
			vTaskDelayUntil(&lastwake, freq);
		}
	}

}

void Player_Task(void * pvParameters)
{
	int pos = 0;
	static _Bool prnt_flg = 0;
	TickType_t lastwake = 0;
	TickType_t stop_wait = pdMS_TO_TICKS(100);
	TickType_t debounce = pdMS_TO_TICKS(70);

	TIM3->CCR2 = 42;

	while(1)
	{
		if(!servoP.cal && servoP.currState != stopped)
		{
			if(!prnt_flg)
			{
				printf("CAL2\n\r");
				prnt_flg = 1;
			}

			while(HAL_GPIO_ReadPin(SHLD_A1_GPIO_Port, SHLD_A1_Pin)==GPIO_PIN_RESET)
			{
				TIM3->CCR2 -= 5;
				lastwake = xTaskGetTickCount();
				vTaskDelayUntil(&lastwake, stop_wait);
			}

			while(HAL_GPIO_ReadPin(SHLD_A3_GPIO_Port, SHLD_A3_Pin)==GPIO_PIN_RESET)
			{
				TIM3->CCR2 += 5;
				lastwake = xTaskGetTickCount();
				vTaskDelayUntil(&lastwake, stop_wait);
			}

			if(servoP.currState == calibratingL)
			{
				if(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET)
				{
					lastwake = xTaskGetTickCount();
					vTaskDelayUntil(&lastwake, debounce);
					if(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET)
					{
						servoP.position[pos0] = TIM3->CCR2;
						printf("Low Measurement Captured\n\r");
						servoP.currState = calibratingR;
					}
				}
			}
			else if(servoP.currState == calibratingR)
			{
				if(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET)
				{
					lastwake = xTaskGetTickCount();
					vTaskDelayUntil(&lastwake, debounce);
					if(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET)
					{
						servoP.position[pos5] = TIM3->CCR2;
						TIM3->CCR2 = servoP.position[pos0];
						printf("High Measurement Captured\n\r");
						servoP.currState = stopped;
						servoP.cal = 1;
						vTaskSuspend(player);
					}
				}
			}
		}
	}
}


void calibration_Task(void * pvParameters)
{
	if(!servoP.cal && servoP.currState != stopped)
	{
		if(!prnt_flg)
		{
			printf("CAL2\n\r");
			prnt_flg = 1;
		}

		while(HAL_GPIO_ReadPin(SHLD_A1_GPIO_Port, SHLD_A1_Pin)==GPIO_PIN_RESET)
		{
			TIM3->CCR2 -= 5;
			lastwake = xTaskGetTickCount();
			vTaskDelayUntil(&lastwake, stop_wait);
		}

		while(HAL_GPIO_ReadPin(SHLD_A3_GPIO_Port, SHLD_A3_Pin)==GPIO_PIN_RESET)
		{
			TIM3->CCR2 += 5;
			lastwake = xTaskGetTickCount();
			vTaskDelayUntil(&lastwake, stop_wait);
		}

		if(servoP.currState == calibratingL)
		{
			if(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET)
			{
				lastwake = xTaskGetTickCount();
				vTaskDelayUntil(&lastwake, debounce);
				if(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET)
				{
					servoP.position[pos0] = TIM3->CCR2;
					printf("Low Measurement Captured\n\r");
					servoP.currState = calibratingR;
				}
			}
		}
		else if(servoP.currState == calibratingR)
		{
			if(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET)
			{
				lastwake = xTaskGetTickCount();
				vTaskDelayUntil(&lastwake, debounce);
				if(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET)
				{
					servoP.position[pos5] = TIM3->CCR2;
					TIM3->CCR2 = servoP.position[pos0];
					printf("High Measurement Captured\n\r");
					servoP.currState = stopped;
					servoP.cal = 1;
					vTaskSuspend(player);
				}
			}
		}
	}
}
