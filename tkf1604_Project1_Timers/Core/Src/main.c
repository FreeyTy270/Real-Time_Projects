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
#include "string.h"
#include "main.h"
#include "uart.h"
#include "timer.h"

#define MAX_SIZE 88

char message[50];    // message to print to the user
char buffer[20];	     // holds the user response

//////////////////////////////////////////////////////////////
// Function declarations
////////////////
/*
// runs the power on self-test. Returns true if the test passes, false otherwise
_Bool power_on_self_test( void );

// initializes all variables prior to a measurement.
int init_measurement( uint32_t limit );

// measures timing of 1000 rising edges.
int make_measurements( uint32_t limit );

// print the non-zero bucket times and counts
int print_measurements( uint32_t limit );


// Parses a line of user input into a new lower limit (unchanged if no response or invalid response)
void get_limit ( void *buffer, uint32_t *lower_limit );

// initializes the timer (Similar to USART2_Init().  Place in timer.c)
void TIM_Init(void);
*/

//////////////////////////////////////////////////////////////
// Embedded code usually consists of 2 components
//  - The init section is run once at startup and initializes all low level drivers and modules
//  - The main loop runs forever and calls the application tasks repeatedly.
////////////////

uint8_t out_buffer [MAX_SIZE] = {0};
uint8_t in_buffer [MAX_SIZE] = {0};
int n = 0;

int main(void) {
    //uint32_t lower_limit = 1000 - 50;	// the default lower limit in the problem statement

    //////////
    // Initialization executed once at startup
    //////////
	clock_init();
    USART2_Init(9600);
    //TIM_Init();

    //while( power_on_self_test() != 0);

    //////////
    // Main loop runs forever
    //////////
    while(1)
    {

    	 n = sprintf((char *) out_buffer, "What would you like to say?\r\n");
    	 USART_Write(USART2, out_buffer, MAX_SIZE);
    	 memset(out_buffer, 0, MAX_SIZE);

    	 get_line(in_buffer, MAX_SIZE);
    	 n = sprintf((char *) out_buffer, "Is this what you said?\r\n");
    	 USART_Write(USART2, out_buffer, n);
    	 memset(out_buffer, 0, MAX_SIZE);
    	 n = sprintf((char *) out_buffer, "Message: %s\r\n", in_buffer);
    	 USART_Write(USART2, out_buffer, n);
    	 memset(out_buffer, 0, MAX_SIZE);
    	/*
    	// 1. Print “Enter expected period or <CR> if no change”. Wait for user response
    //  print(message);    // Consider using use print code from P0-UART
    	get_line(buffer, sizeof(buffer));

    	// 2. Set a new expected interval time based on user input
    	get_limit(buffer, &lower_limit);

        // 3. measure 1000 pulses
		init_measurement( lower_limit );
		make_measurements( lower_limit );

        // 4. print time/count for non-zero counts
		print_measurements( lower_limit );
		*/
    }
}


//////////////////////////////////////////////////////////////
// Function implementation stubs
////////////////

/*
// runs the power on self-test. Returns true if the test passes, false otherwise
_Bool power_on_self_test( void ) {
	return 0;
}

// initializes all variables prior to a measurement.
int init_measurement( uint32_t limit ) {
	return 0;
}

int make_measurements( uint32_t limit ) {
	return 0;
}

// print the non-zero bucket times and counts
int print_measurements( uint32_t limit ) {
	return 0;
}

// Captures 1 line of text from the console. Returns nul terminated string when \n is entered
void get_line ( void *buffer, int max_length ) {
	return;
}

// Parses a line of user input into a new lower limit (unchanged if no response or invalid response)
void get_limit ( void *buffer, uint32_t *lower_limit ) {
	return;
}

// initializes the timer (Similar to USART2_Init().  Place in timer.c)
void TIM_Init(void) {
	return;
}*/
