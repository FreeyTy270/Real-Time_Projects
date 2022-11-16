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
uint8_t rxbuf = 'n';

sig_t newSig;

enum cmd {
	ch,
	type,
	freq,
	minV,
	maxV,
	noise,

};

unsigned char c_return[] = {'\n', '\r', '>'};
unsigned char caret = '>';
unsigned char clr = '\0';

_Bool cr_flg = 0;
_Bool cmd_flg = 0;

enum cmd Rx_st = ch;


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
		static int index = 0;

		HAL_UART_Transmit(&huart2, &rxbuf, 1, 2);

		if(rxbuf == '\n' || rxbuf == '\r')
		{
			cr_flg = 1;
			index = 0;
			HAL_UART_Transmit(&huart2, c_return, sizeof(c_return), 2);
		}
		else if(rxbuf == ' ')
		{
			if(Rx_st == freq)
			{
				index = 0;
			}
			Rx_st++;
		}
		else
		{
			switch(Rx_st)
			{
			case ch:
				newSig.channel = rxbuf - 1;
				break;
			case type:
				if(rxbuf ==  'R')
				{
					newSig.type = RECT;
				}
				else if(rxbuf == 'S')
				{
					newSig.type = SIN;
				}
				else if(rxbuf == 'T')
				{
					newSig.type = TRI;
				}
				else if(rxbuf == 'A')
				{
					newSig.type = ARB;
				}
				break;
			case freq:
				freqbuf[index] = rxbuf;
				index++;
				break;
			case minV:
			case maxV:
				voltbuf[index] = rxbuf;
				index++;
				break;

			}
		}


}

void read_Task(void * pvParameters)
{

	TickType_t lastWake = 0;
	TickType_t Period = pdMS_TO_TICKS(20);

	HAL_UART_Transmit(&huart2, &caret, sizeof(caret), 2);

	while(1)
	{
		HAL_UARTEx_ReceiveToIdle_DMA(&huart2, &rxbuf, 1); // Begin DMA
		__HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);


		if(cr_flg)
		{
			cr_flg = 0;
			newSig.freq = atoi(freqbuf);
			newSig.minV = voltbuf[0] + voltbuf[1]/10;
			newSig.maxV = voltbuf[2] + voltbuf[3]/10;
			printf("Calculating new signal...\n\n\r");
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
