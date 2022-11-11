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

#include "signal.h"

extern UART_HandleTypeDef huart2;
extern QueueHandle_t msgQ;

uint8_t mainbuf[2] = {0};
uint8_t rxbuf = 'n';

extern QueueHandle_t msgQ;

unsigned char c_return[] = {'\n', '\r', '>'};
unsigned char caret = '>';
unsigned char clr = '\0';

_Bool cr_flg = 0;


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
		static int index = 0;

		HAL_UART_Transmit(&huart2, &rxbuf, 1, 2);
}

void read_Task(void * pvParameters)
{
	while(1)
	{
		vTaskDelay(5);

	}
}
