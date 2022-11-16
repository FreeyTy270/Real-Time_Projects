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

#include "uart.h"
#include "signal.h"

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern QueueHandle_t msgQ;
extern TaskHandle_t rdr;

uint8_t freqbuf[5] = {0};
uint8_t voltbuf[4] = {0};
char sig_type[4];
uint8_t rxbuf = 'n';
int rightofone = 0;

sig_t newSig;

enum cmd {
	ch,
	type,
	freq,
	minV,
	maxV,
	noise,

};

unsigned char caret[] = "\n\r> ";
unsigned char clr = '\0';

_Bool cr_flg = 0;
_Bool cmd_flg = 0;
_Bool prd_flg = 0;

enum cmd Rx_st = ch;

/*Callback for UART receiver. Every 1 character triggers this callback function which does light processing of value*/
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
		static int index = 0;

		HAL_UART_Transmit(&huart2, &rxbuf, 1, 2); //Echo character

		if(rxbuf == '\n' || rxbuf == '\r') //If enter key has been hit
		{
			cr_flg = 1; //Set the flag which is checked in the reader task below
			index = 0; //Clear the array index being used for frequency and voltage buffers
			Rx_st = 0; // Reset receiver state variable
			newSig.freq = 0; //Prime new signal for calculations
		}
		else if(rxbuf == ' ') //Receiver state changed when user hits space bar
		{
			if(Rx_st == freq) //Reset index for voltage buffer
			{
				if(!prd_flg)
				{
					rightofone = index;
				}
				index = 0;
			}
			else if(Rx_st == minV && index != 2) //If only one value entered for either minV or maxV, assume user wanted whole number
			{
				voltbuf[1] = '0';
				index++;
			}
			else if(Rx_st == maxV && index != 4)
			{
				voltbuf[3] = '0';
			}
			Rx_st++; //Begin looking for next value on cmd line
		}
		else
		{
			switch(Rx_st)
			{
			case ch:
				newSig.channel = (rxbuf - 48) - 1; //channel is a boolean variable for size, 0 for ch 1, 1 for ch 2. Subtract 48 to get int value from char
				break;
			case type: //Set signal type in sig structure and for printing based on single character input
				if(rxbuf ==  'R')
				{
					newSig.type = RECT;
					strcpy(sig_type, "SQR");
				}
				else if(rxbuf == 'S')
				{
					newSig.type = SIN;
					strcpy(sig_type, "SIN");
				}
				else if(rxbuf == 'T')
				{
					newSig.type = TRI;
					strcpy(sig_type, "TRI");
				}
				else if(rxbuf == 'A')
				{
					newSig.type = ARB;
					strcpy(sig_type, "ARB");
				}
				break;
			case freq: //Fill frequency buffer with incoming values. Conversion to numerical value handled elsewhere
				if(rxbuf == '.')
				{
					prd_flg = 1; //Signal reader that there is a floating point value in the buffer
					rightofone = index; //Record which index holds this value
				}
				else
				{
					freqbuf[index] = rxbuf - 48; // Save numerical value of input
					index++; //Look for next frequency value
				}
				break;
			case minV:
			case maxV:
				if(rxbuf != '.')
				{
					voltbuf[index] = rxbuf;
					index++;
				}
				break;
			case noise:
				newSig.noise = rxbuf - 48;
				break;

			}
		}


}

void read_Task(void * pvParameters)
{

	TickType_t lastWake = 0;
	TickType_t Period = pdMS_TO_TICKS(20);

	HAL_UART_Transmit(&huart2, caret, sizeof(caret), 2);

	while(1)
	{
		HAL_UARTEx_ReceiveToIdle_DMA(&huart2, &rxbuf, 1); // Begin DMA
		__HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);


		if(cr_flg)
		{
			cr_flg = 0;
			for(int i = rightofone-1; i >= 0; i--)
			{
				newSig.freq += pow(10, i)*freqbuf[rightofone-1-i];
			}
			if(prd_flg)
			{
				prd_flg = 0;
				newSig.freq += freqbuf[rightofone]/10.0;
			}
				newSig.minV = (voltbuf[0] - 48.0) + (voltbuf[1] - 48.0)/10;
			newSig.maxV = (voltbuf[2] - 48.0) + (voltbuf[3] - 48.0)/10;

			for(int i = 0; i < 5; i++)
			{
				freqbuf[i] = 0;
			}
			for(int i = 0; i < 4; i++)
			{
				voltbuf[i] = 0;
			}

			printf("Calculating new signal...\n\n\r");
			printf("******* SIG METRICS *******\n\r");
			printf("Signal Type: %s\n\r", sig_type);
			printf("New Frequency: %.1f\n\r", newSig.freq);
			printf("DAC Update Frequency: %.2f\n\r", newSig.freq*Fs);
			printf("Number of samples per period: %d\n\r", Fs);
			printf("Max DAC Value: %d\tMin DAC Value: %d\n\r", (int) (newSig.maxV/3.3*4096), (int) (newSig.minV/3.3*4096));
			HAL_UART_Transmit(&huart2, caret, sizeof(caret), 2);
			cmd_flg = 1;

			if(xQueueSend(msgQ, &newSig, Period) != pdTRUE)
			{
				printf("Could not post new signal to mailbox\n\r");
			}
		}

		lastWake = xTaskGetTickCount();
		vTaskDelayUntil(&lastWake, Period);




	}
}
