/*
 * uart.c
 *
 *  Created on: Sep 26, 2022
 *      Author: Ty Freeman
 */

#include "uart.h"
#include <stdio.h>
#include "string.h"

uint8_t mainbuf[2] = {0};
uint8_t rxbuf = 'n';

unsigned char c_return[] = {'\n', '\r', '>'};
unsigned char caret = '>';
unsigned char clr = '\0';

_Bool cr_flg = 0;

void Buf_Init(void) {
	memset(mainbuf, '\0', 10);
	HAL_UART_Transmit(&huart2, &caret, sizeof(caret), 2);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size) {
	static int index = 0;

	HAL_UART_Transmit(&huart2, &rxbuf, 1, 2);

	if (huart->Instance == USART2)
	{
		if (rxbuf == '\r')
		{
			cr_flg = 1;
			HAL_UART_Transmit(&huart2, c_return, sizeof(c_return), 2);
		}
		else if(rxbuf == '\b')
		{
			HAL_UART_Transmit(&huart2, &clr, sizeof(clr), 2);
		}
		else if (rxbuf == 'N' || rxbuf == 'n')
		{
			mainbuf[index] = 'N';
		}
		else if (rxbuf == 'C' || rxbuf == 'c')
		{
			mainbuf[index] = 'C';
		}
		else if (rxbuf == 'R' || rxbuf == 'r')
		{
			mainbuf[index] = 'R';
		}
		else if (rxbuf == 'P' || rxbuf == 'p')
		{
			mainbuf[index] = 'P';
		}
		else if (rxbuf == 'B' || rxbuf == 'b')
		{
			mainbuf[index] = 'B';
		}
		else if (rxbuf == 'S' || rxbuf == 's')
		{
			mainbuf[index] = 'S';
		}

		if(index == 1)
		{
			index = 0;
		}
		else
		{
			index ++;
		}

		rxbuf = 'n';
	}
}

