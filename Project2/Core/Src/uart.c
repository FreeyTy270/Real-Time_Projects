/*
 * uart.c
 *
 *  Created on: Sep 26, 2022
 *      Author: Ty Freeman
 */

#include "string.h"
#include "uart.h"
#include "stm32l4xx_hal.h"

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;

#define UART huart2
#define DMA hdma_usart2_rx

#define RXBUF_SIZE 10
#define MAINBUF_SIZE 20

uint8_t rxbuf[] = {'n','n'};
uint8_t mainbuf[MAINBUF_SIZE];

void Buf_Init(void)
{
	memset(rxbuf, '\0', RXBUF_SIZE);
	memset(mainbuf, '\0', MAINBUF_SIZE);

	HAL_UARTEx_ReceiveToIdle_DMA(&UART, rxbuf, RXBUF_SIZE);
	__HAL_DMA_DISABLE_IT(&DMA, DMA_IT_HT);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
	if (huart->Instance == USART2)
	{
		if(rxbuf)
		memcpy(mainbuf, rxbuf, size);
		memset(mainbuf, '\0', MAINBUF_SIZE);
		HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rxbuf, RXBUF_SIZE);
		__HAL_DMA_DISABLE_IT(&DMA, DMA_IT_HT);


	}
}




