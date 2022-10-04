/*
 * uart.c
 *
 *  Created on: Sep 26, 2022
 *      Author: Ty Freeman
 */

#include "uart.h"
#include "string.h"
#include "stm32l4xx_hal.h"

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;

#define UART huart2
#define DMA hdma_usart2_rx

uint8_t rxbuf = 'n';
uint8_t mainbuf[10] = {0};

_Bool cr_flg = 0;


void Buf_Init(void)
{
	memset(mainbuf, '\0',10);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
	uint8_t resp = 0;
	static int index = 0;

	printf("%c", rxbuf)

	if (huart->Instance == USART2)
	{
		if(rxbuf[0] == '\n')
		{
			cr_flg = 1;
			resp = '\n>';
		}
		if(rxbuf[0] == 'n' || rxbuf[0] == 'N')
		{
			resp = 'N';
		}
		else if(rxbuf[0] == 'C' || rxbuf[0] == 'c')
		{
			resp = 'C';
		}
		else if(rxbuf[0] == 'R' || rxbuf[0] == 'r')
		{
			resp = 'R';
		}
		else if(rxbuf[0] == 'P' || rxbuf[0] == 'p')
		{
			resp = 'P';
		}
		else if(rxbuf[0] == 'B' || rxbuf[0] == 'b')
		{
			resp = 'B';
		}

		mainbuf[index] = resp;
		index += 1;

		memset(rxbuf, '\0', 1);
	}
}




