/*
 * time_calc.c
 *
 *  Created on: Oct 19, 2022
 *      Author: Ty Freeman
 */

#include <numbers.h>
#include "stm32l4xx_hal.h"

extern RNG_HandleTypeDef hrng;


/* Segment byte maps for numbers 0 to 9 */

const char SEGMENT_MAP[] = {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0X80,0X90};
/* Byte maps to select digit 1 to 4 */
const char SEGMENT_SELECT[] = {0xF1,0xF2,0xF4,0xF8};


int num_gen(reas_t reason)
{
	uint32_t randNum;
	int mask;
	int max;
	int min;


	HAL_RNG_GenerateRandomNumber(&hrng, &randNum);
	switch(reason)
	{
	case (mngr):
	case (break_len):
		mask = 0xFF;
		min = 60;
		max = 240;
		break;
	case process:
		mask = 0x1FF;
		min = 30;
		max = 480;
		break;
	case break_start:
		mask = 0xFFF;
		min = 1800;
		max = 3600;
		break;
	}
	randNum = randNum & mask;

	if(randNum > max)
	{
		randNum = max;
	}
	else if(randNum < min)
	{
		randNum = min;
	}

	return randNum;
}
void format_time(int time_since, int *hr, int *min)
{
	int hr_since = 0;
	int min_since = 0;

	min_since = time_since / 60;
	hr_since = min_since / 60;

	*hr = (hr_since > 3) ? hr_since - 3 : START + hr_since;
	*min = (min_since % 60);

}

void shiftOut(uint8_t val)
{
      for(int ii=0x80; ii; ii>>=1) {
    	  HAL_GPIO_WritePin(SHLD_D7_SEG7_Clock_GPIO_Port,SHLD_D7_SEG7_Clock_Pin, GPIO_PIN_RESET);    // clear clock pin
      		if(ii & val)						                                                     // if this bit in `value` is set
      			HAL_GPIO_WritePin(SHLD_D8_SEG7_Data_GPIO_Port, SHLD_D8_SEG7_Data_Pin,GPIO_PIN_SET);  //   set it in shift register
      		else
      			HAL_GPIO_WritePin(SHLD_D8_SEG7_Data_GPIO_Port, SHLD_D8_SEG7_Data_Pin,GPIO_PIN_RESET); 	//   else clear it

      		HAL_GPIO_WritePin(SHLD_D7_SEG7_Clock_GPIO_Port,SHLD_D7_SEG7_Clock_Pin, GPIO_PIN_SET);       // set clock pin
      	}
}

/* Write a decimal number between 0 and 9 to one of the 4 digits of the display */
void WriteNumberToSegment(int Segment, int Value)
{
  HAL_GPIO_WritePin(SHLD_D4_SEG7_Latch_GPIO_Port, SHLD_D4_SEG7_Latch_Pin, GPIO_PIN_RESET);
  shiftOut(SEGMENT_MAP[Value]);
  shiftOut(SEGMENT_SELECT[Segment] );
  HAL_GPIO_WritePin(SHLD_D4_SEG7_Latch_GPIO_Port, SHLD_D4_SEG7_Latch_Pin, GPIO_PIN_SET);
}

void dig_ret(unsigned long val, int *digBuf)
{
	int i = 0;
	while(val > 0)
	{

		int mod = val % 10;  //split last digit from number
		digBuf[i] = mod; //print the digit.
		i++;

		val = val / 10;    //divide num by 10. num /= 10 also a valid one
	}
}

stats_t calcs(int *timeBuf, _Bool teller)
{
	stats_t stat;
	int len = 200;
	int size;
	int sum = 0;

	stat.max = timeBuf[0];

	if(teller)
	{
		size = 20;
	}
	else
	{
		size = 200;
	}

	for(int i = 0; i < size; i++)
	{
		if(timeBuf[i] > 0)
		{
			if(timeBuf[i] > stat.max)
			{
				stat.max = timeBuf[i];
			}
			sum += timeBuf[i];
		}
		else
		{
			len--;
		}
	}

	stat.ave = sum/len;

	return stat;

}
