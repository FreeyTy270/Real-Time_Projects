/*
 * ind_level.c
 *
 *  Created on: Oct 7, 2022
 *      Author: Ty Freeman
 *
 *      File for all functions related to individual servo motors. The middle layer section
 */

#include "stm32l4xx.h"
#include "ind_level.h"
#include "low_level.h"

extern uint8_t *recipes[];

/* Servo device structure declaration */
typedef struct servo
{
	int dev;
	int position;
	int status;
	int nxt_event;
	uint8_t *recipe;
	int recipe_index;
	opcode_t old_com;
	opcode_t new_com;
	int loop_flg : 3;
}servo_t;

/* Create two servo objects representing the current system. Set device id, unknown position, paused status and default (none) next event request */
servo_t servo1 = {1, serv_unknown, status_paused, nothing};
servo_t servo2 = {2, serv_unknown, status_paused, nothing};

/*********************************************************************************************
 * 																							 *
 ** Individual servo initialization. Home's servo motor to position 1 and sets motor recipe **
 * 																							 *
 *********************************************************************************************/
void go(servo_t *servo)
{
	int delay = 0;
	servo->new_com.operation = MOV; // Prepare servo to home to position one by setting the command
	servo->new_com.data = 1;

	/* Set initial servo recipes based on device id */
	if(servo->dev == 1)
	{
		servo->recipe = recipes[0];
	}
	else
	{
		servo->recipe = recipes[1];
	}

	/*
	 * Assume worst case scenario as farthest distance from "home" and then set current state to run while movement takes place.
	 * once movement has completed reinitialize servo by setting it in paused mode and clearing servo flags.
	 */
	servo->position = serv_pos5;
	servo->status = status_running;
	delay = run_inst(servo->dev, servo->position, servo->new_com);
	HAL_Delay(delay);
	servo->position = serv_pos1;
	servo->status = status_paused;
	servo->recipe_index = 0;
	servo->loop_flg = 0;
}

/***********************************************************************************
 * 																				   *
 ** Processes user input override and sets servo's next event request accordingly **
 * 																				   *
 ***********************************************************************************/
void override_process(servo_t *servo, uint8_t cmd)
{
	switch(cmd)
	{
	case 'N': // If no command then the nxt_event should remain the same as it was previously
		break;
	case 'C':
		servo->nxt_event = cont;
		break;
	case 'R':
		servo->nxt_event = right;
		break;
	case 'L':
		servo->nxt_event = left;
		break;
	case 'P':
		servo->nxt_event = stop;
		break;
	case 'B':
		servo->nxt_event = start;
		break;
	case 'S':
		servo->nxt_event = swap;
	}
}
/********************************************************************************************************************************
* 																															    *
** Checks legality of servo's next event request against the current state and then sets the new state if request is granted. **
* 																															    *
********************************************************************************************************************************/
int chk_state(servo_t *servo)
{
	_Bool response = 0;
	_Bool user_mov = 0;

	if(servo->nxt_event == left || servo->nxt_event == right) // Differentiates a request to move from a request to continue
	{
		user_mov = 1;
	}

	if(servo->nxt_event == start) // Begin request is not dependent on state
	{
		servo->status = status_running;
		servo->recipe_index = 0;
		servo->nxt_event = cont;
	}
	else if(servo->nxt_event == stop && servo->status == status_running && servo->position != recipe_ended) // If the servo is requested to stop while it is running and the recipe is not over then it will enter the paused state
	{
		servo->status = status_paused;
	}
	else if(servo->nxt_event == cont && servo->status == status_paused && servo->position != recipe_ended) // If the servo is requested to continue after being paused and the recipe has not ended then the servo will begin running again
	{
		servo->status = status_running;
	}
	else if(servo->nxt_event == swap && servo->status == status_paused && servo->position != recipe_ended) // If the servo is requested to swap recipe sets with the other servo and the servos are currently paused and its current recipe hasn't ended then the servo will begin the process of swapping recipes
	{
		servo->status = status_swapping;
	}
	else if(user_mov && servo->status == status_paused) // If the servo is told to move left or right and it is currently paused then the servo will run as long as the current position does not interfere
	{
		if(servo->nxt_event == left && servo->position > 1)
		{
			servo->status = status_running;
		}
		else if(servo->nxt_event == right && servo->position < 5)
		{
			servo->status = status_running;
		}
		else
		{
			servo->status = status_cmd_error;
		}
	}
	else // Default to status remaining unchanged if the event request is nothing or the request/state combination is illegal and the servo enters a cmd error state
	{
		if(servo->nxt_event != nothing)
		{
			servo->status = status_cmd_error;
		}
	}

/*
 * Checks servo status to return servo response flags (signals servo will be holding for next run)
 * Also sets LED's according to servo 1's status
 */
	if(servo->status == status_cmd_error)
	{
		response = 1;
		if(servo->dev == 1)
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0);
		}
	}
	else if(servo->status == status_paused)
	{
		response = 1;
		if(servo->dev == 1)
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_6, 0);
		}

	}
	else if(servo->status == status_running)
	{
		response = 0;

		if(servo->dev == 1)
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);
		}

		if(user_mov)
		{
			servo->status = status_paused;
		}
	}

	return response;
}

/******************************************************************************************************
* 																								      *
** Fetches the next command for each servo based on the nxt_event request and the servo's loop flag. **
**  This routine is only called if the servo status is status_running. If not a user called movement **
**  					the servo will run according to current looping status.						 **																									    *																									  *
******************************************************************************************************/
void fetch_next(servo_t *servo)
{
	int loop_start = 0;
	static int nrepeat = 0;

	static int new_index = 0;

	if(servo->nxt_event == left) //
	{
		servo->old_com = servo->new_com;
		servo->new_com.operation = MOV;
		servo->new_com.data = (servo->position - 1);
	}
	else if(servo->nxt_event == right)
	{
		servo->old_com = servo->new_com;
		servo->new_com.operation = MOV;
		servo->new_com.data = (servo->position + 1);
	}
	else
	{
		switch(servo->loop_flg) // Processing of loop flag for each stage of loop process
		{
			case 0: // Default: Loop has not been called run as normal.
				servo->old_com = servo->new_com;
				servo->new_com = read_recipe(servo->recipe, servo->recipe_index);
				servo->recipe_index++;
				break;
			case 1: // Loop command read from recipe. Index saved for returning to at END_LOOP and the number of times repeated also saved.
				servo->loop_flg = 2;
				loop_start = servo->recipe_index + 1;
				nrepeat = servo->new_com.data;
				new_index = loop_start;
				break;
			case 2: // Loop status has been set, each read recipe is now checking for loop errors
				servo->old_com = servo->new_com;
				servo->new_com = read_recipe(servo->recipe, servo->recipe_index);
				if(servo->new_com.operation == LOOP)
				{
					servo->status = status_loop_error;
					servo->new_com.operation = WAIT;
					servo->new_com.data = 0;
					if(servo->dev == 1)
					{
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_6, 1);

					}
				}
				servo->recipe_index++;
			case 3: // END_LOOP read from read_recipe. If nrepeat satisfied then the servo will exit looping mode
				if(nrepeat == 0)
				{
					servo->loop_flg = 0;
					break;
				}
				nrepeat--;
				servo->loop_flg += 1; // Otherwise it will enter looping mode. Returning to the loop beginning index saved in new_index
			case 4: // Run through instructions starting at Loop command + 1 (First command of loop)
				servo->new_com = read_recipe(servo->recipe, new_index);
				new_index++;
				break;
		}
	}
}

/***************************************************************************************************
* 																								   *
** Calls the run_instruction command for each servo, also calls routine to process returned delay **
* 																								   *
****************************************************************************************************/
int run_servo(servo_t *servo)
{
	int delay = run_inst(servo->dev, servo->position, servo->new_com);

	chk_delay(servo, delay);

	return delay;

}

/**********************************************************************************
* 																				  *
** Processes information encoded in delay returned from run_instruction routine. **
* 																				  *
***********************************************************************************/
void chk_delay(servo_t *servo, const int delay)
{
	if(delay > 31 && delay < 35) // Special delay information is greater than max WAIT command delay lengths but less than modified delay lengths from servo move commands
		{
			switch(delay) // Loop logic processed in fetch_next routine.
			{
			case 32: // Loop has begun
				servo->loop_flg = 1;
				break;
			case 33: // END_LOOP has been read from recipe
				servo->loop_flg = 3;
				break;
			case 34: // END_RECIPE read from recipe
				servo->position = recipe_ended;
				break;
			}
		}
}

/************************************************************
* 															*
** Swap recipe pointer of servos depending on user inputs. **
* 															*
************************************************************/
void swap_recipe(servo_t *servo1, servo_t *servo2, int swap_flg)
{
	uint8_t *temp;

	if(swap_flg == 3) // Both servos were requested to be swapped
	{
		temp = servo1->recipe;
		servo2->recipe = servo1->recipe;
		servo2->recipe = temp;
	}
	else if(swap_flg == 2) // User wants servo 2 to now operate with servo 1's recipe
	{
		servo2->recipe = servo1->recipe;
	}
	else if(swap_flg == 1) // User wants servo 1 to now operate with servo 2's recipe
	{
		servo1->recipe = servo2->recipe;
	}
}

/*******************************************************************************************
* 																						   *
** Servo status not running and will instead hold for one cycle waiting for user override **
* 																						   *
*******************************************************************************************/
void hold(servo_t *servo)
{
	servo->old_com = servo->new_com;

	servo->new_com.operation = WAIT;
	servo->new_com.data = 0;
}

void set_servo(servo_t *servo, int param, int val)
{
	switch(param)
	{
	case p_position:
		servo->position = val;
		break;
	case p_status:
		servo->status = val;
		break;
	case p_nxt_event:
		servo->nxt_event = val;
		break;
	case p_recipe_index:
		servo->recipe_index = val;
		break;
	}
}
