/*
 * time_calc.h
 *
 *  Created on: Oct 19, 2022
 *      Author: Ty Freeman
 */

#ifndef INC_NUMBERS_H_
#define INC_NUMBERS_H_
#include <inttypes.h>

#define START 9
#define END 4

#define SHLD_D7_SEG7_Clock_Pin GPIO_PIN_8
#define SHLD_D7_SEG7_Clock_GPIO_Port GPIOA
#define SHLD_D8_SEG7_Data_Pin GPIO_PIN_9
#define SHLD_D8_SEG7_Data_GPIO_Port GPIOA
#define SHLD_D4_SEG7_Latch_Pin GPIO_PIN_5
#define SHLD_D4_SEG7_Latch_GPIO_Port GPIOB

typedef enum reasons{
	mngr,
	process,
	break_start,
	break_len
}reas_t;

typedef struct stats{
	int ave;
	int max;
}stats_t;


int num_gen(reas_t reason);
void format_time(int time_since, int *hr, int *min);
void shiftOut(uint8_t val);
void WriteNumberToSegment(int Segment, int Value);
void dig_ret(unsigned long val, int *digBuf);
stats_t calcs(int *timeBuf, _Bool teller);


#endif /* INC_NUMBERS_H_ */
