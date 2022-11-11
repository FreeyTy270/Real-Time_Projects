/*
 * uart.h
 *
 *  Created on: Nov 10, 2022
 *      Author: Ty Freeman
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#define UART huart2
#define DMA hdma_usart2_rx

void read_Task(void * pvParameters);

#endif /* INC_UART_H_ */
