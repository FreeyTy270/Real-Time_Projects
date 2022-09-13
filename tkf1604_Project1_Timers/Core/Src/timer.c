/*
 * timer.c
 *
 *  Created on: Aug 31, 2022
 *      Author: Ty Freeman
 */

#include <timer.h>

static void TIM_init( void )
{
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;	// enable clock for this timer in the clock control
	TIM2->PSC = 15999;						// load the prescaler value -- divide 16 MHz clock down to 1 kHz
	TIM2->EGR |= TIM_EGR_UG ;				// force an update event to make the prescaler load take effect
	TIM2->CR1 |= TIM_CR1_CEN ;				// now start the timer*/


}
