/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "main.h"
#include "uart.h"
#include "timer.h"

#define MAX_SIZE 88

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  clock_init();
  USART2_Init(115200);
  //TIM2_init();


  uint8_t out_buffer [MAX_SIZE] = {0};
  uint8_t in_buffer [MAX_SIZE] = {0};
  int n = 0;

  while (1)
  {
	  n = sprintf((char *) out_buffer, "What would you like to say?\r\n");
	  USART_Write(USART2, out_buffer, n);

	  get_line(in_buffer, MAX_SIZE);
	  n = sprintf((char *) out_buffer, "Is this what you said?\r\n");
	  USART_Write(USART2, out_buffer, n);
	  n = sprintf((char *) out_buffer, "Message: %s\r\n", in_buffer);
	  USART_Write(USART2, out_buffer, n);
  }
}

