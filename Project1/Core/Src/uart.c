/*
 * uart.c
 *
 *  Created on: Sep 13, 2022
 *      Author: Ty Freeman
 */
#include "stm32l4xx.h"
#include "string.h"

extern UART_HandleTypeDef huart2;

void get_line(unsigned char *buffer, int max_size)
{
	unsigned char new_char = '\0';
	int i = 0;

	memset(buffer,0,max_size);

	do
	{
		HAL_UART_Receive(&huart2, &new_char, sizeof(new_char), HAL_MAX_DELAY);

		if(new_char == '\b' && i > 0)
		{
			i--;
			buffer[i] = new_char;
			//USART_Write(USART2, &new_char, sizeof(new_char));
		}
		else
		{
			buffer[i] = new_char;
			i++;
			//USART_Write(USART2, &new_char, sizeof(new_char));
		}

		//(*(unsigned char *) buffer) = new_char;
		HAL_UART_Transmit(&huart2, &new_char, sizeof(new_char), HAL_MAX_DELAY);

	}while(new_char != '\r' && i < max_size);
}
