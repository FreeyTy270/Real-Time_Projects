/*
 * uart.h
 *
 *  Created on: Sep 26, 2022
 *      Author: Ty Freeman
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "stm32l4xx_hal.h"
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;

#define UART huart2
#define DMA hdma_usart2_rx
/* Initializes used buffers to zero and also initializes the DMA */
void Buf_Init(void);

#endif /* INC_UART_H_ */
