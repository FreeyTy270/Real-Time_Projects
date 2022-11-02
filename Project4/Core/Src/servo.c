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
extern flags_t flgs;

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
	int go_time = 0;
	int dist = 0;

	TickType_t lastwake = 0;
	TickType_t go_time_T = 0;
	TickType_t travel_time_T = 0;

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

		TIM3->CCR1 = servoN.position[pos];
		dist = (servoN.currPos > pos) ? servoN.currPos - pos : pos - servoN.currPos;
		if(dist == 0)
		{
			if(servoN.currPos == pos5)
			{
				pos--;
			}
			else
			{
				pos++;
			}
		}
		servoN.currPos = pos;
		lastwake = xTaskGetTickCount();
		travel_time_T = pdMS_TO_TICKS(100*(dist) + 1);

		vTaskDelayUntil(&lastwake, travel_time_T);

		HAL_RNG_GenerateRandomNumber(&hrng, &rndNum);
		go_time = rndNum & 0xFFF;

		if(go_time > 4000)
		{
			go_time = 4000;
		}
		else if(go_time < 1000)
		{
			go_time = 1000;
		}

		lastwake = xTaskGetTickCount();
		go_time_T = pdMS_TO_TICKS(go_time);
		vTaskDelayUntil(&lastwake, go_time_T);
	}

}

void Player_Task(void * pvParameters)
{

	int pos = pos0;

	TickType_t lastwake = 0;
	TickType_t travel_time = pdMS_TO_TICKS(100);
	TickType_t debounce = pdMS_TO_TICKS(20);

	while(1)
	{
		if(HAL_GPIO_ReadPin(SHLD_A1_GPIO_Port, SHLD_A1_Pin)==GPIO_PIN_RESET)
		{
			lastwake = xTaskGetTickCount();
			vTaskDelayUntil(&lastwake, debounce);
			if(HAL_GPIO_ReadPin(SHLD_A1_GPIO_Port, SHLD_A1_Pin)==GPIO_PIN_RESET)
			{
				flgs.mv_left = 1;
			}
		}
		else if(HAL_GPIO_ReadPin(SHLD_A3_GPIO_Port, SHLD_A3_Pin)==GPIO_PIN_RESET)
		{
			lastwake = xTaskGetTickCount();
			vTaskDelayUntil(&lastwake, debounce);
			if(HAL_GPIO_ReadPin(SHLD_A3_GPIO_Port, SHLD_A3_Pin)==GPIO_PIN_RESET)
			{
				flgs.mv_right = 1;
			}
		}

		if(flgs.mv_left)
		{
			if(servoP.currPos > 0)
			{
				servoP.currPos--;
				flgs.mv_left = 0;
			}
		}

		if(flgs.mv_right)
		{
			if(servoP.currPos < 5)
			{
				servoP.currPos++;
				flgs.mv_right = 0;
			}
		}

		TIM3->CCR2 = servoP.position[servoP.currPos];
		lastwake = xTaskGetTickCount();
		vTaskDelayUntil(&lastwake, travel_time);
	}
}

void calibration_Task(void * pvParameters)
{
	int serNum = (int) pvParameters; // Grab servo identifier
	_Bool prnt_flg = 0; // Has the message been printed already
	TickType_t lastwake = 0;
	TickType_t stop_wait = pdMS_TO_TICKS(100);
	TickType_t debounce = pdMS_TO_TICKS(90);

	servo_t *currServo = NULL; // Container for servo object
	volatile uint32_t *currReg = NULL; // Container for corresponding CCR


	switch(serNum)
	{
	case 1:
		currServo = &servoN;
		currReg = &TIM3->CCR1;
		break;
	case 2:
		currServo = &servoP;
		currReg = &TIM3->CCR2;
	}

	*currReg = 42; // Guess at position zero

	while(1)
	{
		if(!currServo->cal) // If the current servo isn't calibrated and currently searching for its furthest left value
		{
			if(!prnt_flg)
			{
				printf("CAL%d\n\r", serNum); // Print calibration message
				prnt_flg = 1; // Signal message has been printed so it only does it once
			}

			/*Moving the servos left and right in small increments*/
			while(HAL_GPIO_ReadPin(SHLD_A1_GPIO_Port, SHLD_A1_Pin)==GPIO_PIN_RESET)
			{
				*currReg -= 3;
				lastwake = xTaskGetTickCount();
				vTaskDelayUntil(&lastwake, stop_wait);
			}

			while(HAL_GPIO_ReadPin(SHLD_A3_GPIO_Port, SHLD_A3_Pin)==GPIO_PIN_RESET)
			{
				*currReg += 3;
				lastwake = xTaskGetTickCount();
				vTaskDelayUntil(&lastwake, stop_wait);
			}

			/*When Button two is pressed the
			 * state is used to determine
			 * is this is left or right and
			 * then saves the register value accordingly*/

			if(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET)
			{
				lastwake = xTaskGetTickCount();
				vTaskDelayUntil(&lastwake, debounce);
				if(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET)
				{
					if(currServo->currState == calibratingL) // Left edge
					{
						currServo->position[pos0] = *currReg; // Save the value
						printf("Low Measurement Captured\n\r");
						currServo->currState = calibratingR; // Start searching for right edge
						*currReg += 80; // Make estimate
					}
					else if(currServo->currState == calibratingR) // Right edge
					{
						currServo->position[pos5] = *currReg; // Save the value
						*currReg = currServo->position[pos0]; // Move servo back to position zerio
						currServo->currPos = pos0;
						printf("High Measurement Captured\n\r");
						currServo->currState = stopped; // Stop the servo
						currServo->cal = 1; // current servo now calibrated

						vTaskDelete(NULL); // Delete the initialization task
					}
				}
			}
		}
	}
}
