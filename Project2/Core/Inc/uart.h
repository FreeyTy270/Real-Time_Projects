/*
 * uart.h
 *
 *  Created on: Sep 26, 2022
 *      Author: Ty Freeman
 */

#ifndef INC_UART_H_
#define INC_UART_H_

/* Initializes used buffers to zero and also initializes the DMA */
void Buf_Init(void);

uint8_t Command_Processor(uint8_t command);

#endif /* INC_UART_H_ */
