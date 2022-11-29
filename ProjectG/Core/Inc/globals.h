/*
 * globals.h
 *
 *  Created on: Nov 25, 2022
 *      Author: Ty Freeman
 */

#ifndef INC_GLOBALS_H_
#define INC_GLOBALS_H_

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

DAC_HandleTypeDef hdac1;
DMA_HandleTypeDef hdma_dac_ch1;
DMA_HandleTypeDef hdma_dac_ch2;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;



_Bool cmd_flg = 0;


#endif /* INC_GLOBALS_H_ */
