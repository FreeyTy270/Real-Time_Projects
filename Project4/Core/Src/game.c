/*
 * game.c
 *
 *  Created on: Oct 31, 2022
 *      Author: Ty Freeman
 */


#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "string.h"
#include "servo.h"
#include "stm32l4xx_hal.h"



servo_t servoN;
servo_t servoP;


