/*
 * uart.c
 *
 *  Created on: Nov 10, 2022
 *      Author: Ty Freeman
 */
#include <stdlib.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "string.h"
#include "math.h"
#include "event_groups.h"
#include "stm32l4xx_hal.h"

#include "globals.h"
#include "uart.h"
#include "adc.h"

TaskHandle_t adc;
extern QueueHandle_t mbx;
extern TaskHandle_t rdr;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern ADC_HandleTypeDef hadc1;

uint8_t nbuf[5] = {0};

uint8_t rxbuf = '\0';
sig_t newSig = {0, 0, 0, 0, 0, 0};

enum cmd {
	dir,
	ch,
	num
};

unsigned char caret[] = "\n\r> ";
unsigned char cr[] = "\n\r";
unsigned char clr = '\0';

enum cmd Rx_st = dir;

int attempts = 3;

_Bool full = 0;
_Bool cr_flg = 0;
_Bool cmd_flg = 0;
_Bool rd_flg = 0;
_Bool out_flg = 0;
_Bool adc_done = 0;

/*Callback for UART receiver. Every 1 character triggers this callback function which does light processing of value*/
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
	static int index = 0;

	HAL_UART_Transmit(&huart2, &rxbuf, 1, 2); //Echo character

	if(!cr_flg)
	{
		if(rxbuf == '\n' || rxbuf == '\r') //If enter key has been hit
		{
			cr_flg = 1; //Set the flag which is checked in the reader task below
			index = 0; //Clear the array index being used for frequency and voltage buffers
			Rx_st = dir; // Reset receiver state variable

			HAL_UART_Transmit(&huart2, cr, sizeof(cr), 2); //Echo character
		}
		else if(rxbuf == ' ' && out_flg) //Receiver state changed when user hits space bar
		{
			Rx_st++; //Begin looking for next value on cmd line
		}
		else
		{
			switch(Rx_st)
			{
			case dir:
				if(rxbuf == 'c' || rxbuf == 'C')
					rd_flg = 1;
				else if(rxbuf == 'o' || rxbuf == 'O')
					out_flg = 1;
				break;
			case ch:
				newSig.channel = (rxbuf - 48) - 1; //channel is a boolean variable for size, 0 for ch 1, 1 for ch 2. Subtract 48 to get int value from char
				break;
			case num: //Fill buffer with incoming values
				nbuf[index] = rxbuf - 48; // Save numerical value of input
				index++; //Look for next frequency value
				break;
			}
		}
	}
}

void read_Task(void * pvParameters)
{

	TickType_t lastWake = 0;
	TickType_t Period = pdMS_TO_TICKS(20);
	TickType_t wait = pdMS_TO_TICKS(2300);

	double maxV = 0;
	double minV = 0;

	HAL_UART_Transmit(&huart2, caret, sizeof(caret), 2);

	while(1)
	{
		HAL_UARTEx_ReceiveToIdle_DMA(&huart2, &rxbuf, 1); // Begin DMA
		__HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);


		if(cr_flg)
		{

			if(rd_flg)
			{
				printf("Capturing signal\n\r");
				newSig.min = 4096;
				newSig.max = 0;
				newSig.freq = 1000;
				xTaskCreate(adc_Task, "adc", 512, NULL, PriorityNormal, &adc);
				lastWake = xTaskGetTickCount();
				vTaskDelayUntil(&lastWake, wait);
				if(adc_done)
				{
					adc_done = 0;
					attempts = 3;
					maxV = (newSig.max * toreal);
					minV = (newSig.min * toreal);
					printf("\n\n******* SIGNAL CAPTURED *******\n\r");
					printf("Type: %c\n\r", newSig.type);
					if(newSig.freq == -1)
						printf("Frequency: N/A\n\r");
					else
						printf("Frequency: %i\n\r", newSig.freq);
					printf("Max Voltage: %.2f\tMin Voltage: %.2f\n\n\r", maxV, minV);
					HAL_UART_Transmit(&huart2, caret, sizeof(caret), 2);
					rd_flg = 0;
				}
				else
				{
					if(!attempts)
					{
						printf("No more attempts remain. Exiting program, please reset...\n");
						exit(1);
					}
					printf("ERROR: Capture took too long. Retrying...\n");
					HAL_ADC_Stop(&hadc1);
					vTaskDelete(adc);
					attempts--;
				}

			}

			else if(out_flg)
			{
				newSig.width = atoi(nbuf);
				if(newSig.width < 400 && newSig.width > 0)
				{
					printf("Not long enough for a clean signal. Showing full period...\n\r");
					newSig.width = 0;
				}
				else if(newSig.width == 0)
					newSig.width = 5000;
				else
					newSig.width /= 200;

				printf("Outputting signal from channel %d...\n\n\r", newSig.channel + 1);
				if(xQueueSend(mbx, &newSig, Period) != pdTRUE)
				{
					printf("Could not post new signal to mailbox\n\r");
				}
				out_flg = 0;
			}

			cr_flg = 0;

		}

		lastWake = xTaskGetTickCount();
		vTaskDelayUntil(&lastWake, Period);




	}
}
