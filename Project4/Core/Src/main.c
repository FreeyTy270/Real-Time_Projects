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
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "string.h"
#include "math.h"

#include "shld_def.h"
#include "servo.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SHLD_D7_SEG7_Clock_Pin GPIO_PIN_8
#define SHLD_D7_SEG7_Clock_GPIO_Port GPIOA
#define SHLD_D8_SEG7_Data_Pin GPIO_PIN_9
#define SHLD_D8_SEG7_Data_GPIO_Port GPIOA
#define SHLD_D4_SEG7_Latch_Pin GPIO_PIN_5
#define SHLD_D4_SEG7_Latch_GPIO_Port GPIOB

/* Segment byte maps for numbers 0 to 9 */

const char SEGMENT_MAP[] = {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0X80,0X90};
/* Byte maps to select digit 1 to 4 */
const char SEGMENT_SELECT[] = {0xF1,0xF2,0xF4,0xF8};
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RNG_HandleTypeDef hrng;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */
TaskHandle_t game_mngr;
TaskHandle_t npc;
TaskHandle_t player;
TaskHandle_t spinner; // 7-seg display
TaskHandle_t npc_init;
TaskHandle_t player_init;

extern servo_t servoN; // Computer servo
extern servo_t servoP; // Player servo

int timer = 0;
int score = 0; // Player Score
int tot_score = 0;
int taskMade = 0; // Records which initialization functions have been created
int digBuf[4] = {0};

flags_t flgs = {0, 0, 0, 0, 0, {0, 0, 0, 0, 0}};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_RNG_Init(void);
static void MX_TIM3_Init(void);

/* USER CODE BEGIN PFP */
void Game_Task(void * pvParameters);
void spinner_Task( void * pvParameters );

void shiftOut(uint8_t val);
void WriteNumberToSegment(int Segment, int Value);
void dig_ret(unsigned long val, int *digBuf);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_RNG_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* USER CODE BEGIN RTOS_THREADS */
  xTaskCreate(Game_Task, "G_Mngr", 1024, NULL, PriorityHigh, &game_mngr); //Create game manager task
  xTaskCreate(spinner_Task, "Spinner", 256, NULL, PriorityLow, &spinner); //Create Task in charge of 7-seg

  vTaskStartScheduler();
  /* USER CODE END RTOS_THREADS */

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RNG Initialization Function
  * @param None
  * @retval None
  */
static void MX_RNG_Init(void)
{

  /* USER CODE BEGIN RNG_Init 0 */

  /* USER CODE END RNG_Init 0 */

  /* USER CODE BEGIN RNG_Init 1 */

  /* USER CODE END RNG_Init 1 */
  hrng.Instance = RNG;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RNG_Init 2 */

  /* USER CODE END RNG_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 1600-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SHLD_A1_Pin SHLD_A2_Pin */
  GPIO_InitStruct.Pin = SHLD_A1_Pin|SHLD_A2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SHLD_A3_Pin */
  GPIO_InitStruct.Pin = SHLD_A3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SHLD_A3_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN 4 */

  /*Configure GPIO pins : SHLD_A5_Pin SHLD_A4_Pin */
  GPIO_InitStruct.Pin = SHLD_A5_Pin|SHLD_A4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : SHLD_A0_Pin SHLD_D2_Pin */
  GPIO_InitStruct.Pin = SHLD_A0_Pin|SHLD_D2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SHLD_D13_Pin SHLD_D12_Pin SHLD_D11_Pin SHLD_D7_SEG7_Clock_Pin */
  GPIO_InitStruct.Pin = SHLD_D13_Pin|SHLD_D12_Pin|SHLD_D11_Pin|SHLD_D7_SEG7_Clock_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SHLD_D6_Pin SHLD_D5_Pin */
  GPIO_InitStruct.Pin = SHLD_D6_Pin|SHLD_D5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SHLD_D9_Pin */
  GPIO_InitStruct.Pin = SHLD_D9_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SHLD_D9_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SHLD_D8_SEG7_Data_Pin */
  GPIO_InitStruct.Pin = SHLD_D8_SEG7_Data_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(SHLD_D8_SEG7_Data_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SHLD_D3_Pin */
  GPIO_InitStruct.Pin = SHLD_D3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SHLD_D3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SHLD_D4_SEG7_Latch_Pin */
  GPIO_InitStruct.Pin = SHLD_D4_SEG7_Latch_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SHLD_D4_SEG7_Latch_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SHLD_D10_Pin */
  GPIO_InitStruct.Pin = SHLD_D10_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SHLD_D10_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SHLD_D15_Pin SHLD_D14_Pin */
  GPIO_InitStruct.Pin = SHLD_D15_Pin|SHLD_D14_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}


void Game_Task(void * pvParameters)
{
	_Bool cal = 0; // System calibration flag
	int range = 0; // Maximum CCR value - Minimum CCR value
	int step = 0; // Step size between PWM CCR values for 6 equal spacings


	TickType_t lastwake = 0;
	TickType_t debounce = pdMS_TO_TICKS(70);

	servoN.currState = calibratingL; // Prime servos for calibration
	servoP.currState = stopped; // The computer servo is calibrated first so the player servo begins in the stopped state

	while(1)
	{

		if(!cal)
		{
			if(!servoN.cal && !servoP.cal && taskMade == 0) // If neither servo has been calibrated and no task has been created
			{
				xTaskCreate(calibration_Task, "n_init", 512, (int *) 1, PriorityNormal, &npc_init); // Create computer initialization task
				taskMade += 1; // Signal task has been created
			}
			else if(servoN.cal && !servoP.cal && taskMade == 1) // Computer servo has been calibrated but only one task has been created
			{
				servoP.currState = calibratingL;
				xTaskCreate(calibration_Task, "p_init", 512, (int *) 2, PriorityNormal, &player_init); // Create player initialization task
				taskMade += 1; // Signal another task has been created
			}

			if(servoN.cal && servoP.cal) // Both servos have been calibrated
			{
				cal = 1; // System has been calibrated

				/*This block of code calculates
				 * the other positions between the
				 * max and min CCR values that
				 * were found during calibration */

				range = servoN.position[pos5] - servoN.position[pos0];
				step = range / 6;
				for(int i = 1; i < 6; i++)
				{
					servoN.position[i] = servoN.position[i-1] + step;
				}

				range = servoP.position[pos5] - servoP.position[pos0];
				step = range / 6;
				for(int i = 1; i < 6; i++)
				{
					servoP.position[i] = servoP.position[i-1] + step;
				}


				printf("\n\rCalibration Finished!\n\n\r----- Press Button 2 to Begin -----\n\n\r"); // Print message for player

			}
		}
		else
		{
			if(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET && taskMade < 4)
			{
				lastwake = xTaskGetTickCount();
				vTaskDelayUntil(&lastwake, debounce); // Wait for the button to settle
				if(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET) // If button still pressed
				{
					xTaskCreate(NPC_Task, "NPC_Servo", 1024, NULL, PriorityNormal, &npc);
					xTaskCreate(Player_Task, "Player_servo", 1024, NULL, PriorityNormal, &player);
					taskMade += 2; // All 4 tasks have been created
					printf("\t  Game Started\n\n\r"); // Begin playeing
					flgs.start = 1; // Signal game started
				}
			}

			if(servoN.currPos == servoP.currPos && flgs.start && !flgs.matched[servoN.round_cnt] && servoN.round_cnt > 0 &&servoN.round_cnt < 6) // If game has begun and servos are at the same position then capture how long it took for the servo to move there
			{
				score = timer;
				printf("Round %d:\tMatch!\tScore: %d\n\r", servoN.round_cnt, score);
				tot_score += score;
				flgs.matched[servoN.round_cnt] = 1;
			}
			else if(servoN.currPos != servoP.currPos && flgs.start && !flgs.matched[servoN.round_cnt - 1])
			{
				printf("Round %d:\tMatch!\tScore: %d\n\r", servoN.round_cnt - 1, 4000);
				tot_score += 4000;
				flgs.matched[servoN.round_cnt - 1] = 1;
			}
		}

		if(servoN.round_cnt == 6)
		{
			printf("\t  ----- GAME OVER -----\n\n\n\n\r");
			printf("Total score: %d\n\n\r", tot_score);
			printf("Thanks for playing!\n\n\r");

			vTaskDelete(NULL);
		}
		lastwake = xTaskGetTickCount();
		vTaskDelayUntil(&lastwake, pdMS_TO_TICKS(10));
	}
}

void spinner_Task( void * pvParameters )
{
	TickType_t disp_ticks = pdMS_TO_TICKS(2);

	  for(;;)
	  {
		  TickType_t prev = xTaskGetTickCount();

		  if(servoN.round_cnt < 6)
		  {
			  dig_ret(score, digBuf);
		  }
		  else
		  {
			  dig_ret(tot_score, digBuf);
		  }

		  WriteNumberToSegment(3, digBuf[0]);
		  vTaskDelayUntil(&prev, disp_ticks);
		  prev = xTaskGetTickCount();
		  WriteNumberToSegment(2, digBuf[1]);
		  vTaskDelayUntil(&prev, disp_ticks);
		  prev = xTaskGetTickCount();
		  WriteNumberToSegment(1, digBuf[2]);
		  vTaskDelayUntil(&prev, disp_ticks);
		  prev = xTaskGetTickCount();
		  WriteNumberToSegment(0, digBuf[3]);
		  vTaskDelayUntil(&prev, disp_ticks);
	  }
}

void shiftOut(uint8_t val)
{
  for(int ii=0x80; ii; ii>>=1)
  {
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

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
  if(flgs.go)
  {
	  timer++;
  }
  else
  {
	  timer = 0;
  }
  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
