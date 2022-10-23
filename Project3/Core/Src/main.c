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
#include <numbers.h>
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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct teller{
	TickType_t greeting_time;
	int done_time;
	int cust_timer;
	int cust_helped;

}teller_t;

teller_t helpDesk[3];

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RNG_HandleTypeDef hrng;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

QueueHandle_t waitingRoom;
SemaphoreHandle_t doorKey;
SemaphoreHandle_t speakingStick;

TaskHandle_t h_mngTask;
TaskHandle_t h_teller1;
TaskHandle_t h_teller2;
TaskHandle_t h_teller3;
TaskHandle_t h_uart;
TaskHandle_t h_spinner;

int cust_cnt = 0;
int cust_helped = 0;
int timer = 0;
int help_times[200] = {0};
int wait_times[200] = {0};
_Bool bad_QPost = 0;
_Bool bad_QRead = 0;

teller_t tell1;
teller_t tell2;
teller_t tell3;

/* Segment byte maps for numbers 0 to 9 */

const char SEGMENT_MAP[] = {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0X80,0X90};
/* Byte maps to select digit 1 to 4 */
const char SEGMENT_SELECT[] = {0xF1,0xF2,0xF4,0xF8};


int dig_buffer[4];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_RNG_Init(void);

/* USER CODE BEGIN PFP */
void mng_Task( void * pvParameters );
void teller_Task( void * pvParameters );
void uart_Task( void * pvParameters );
void spinner_Task( void * pvParameters );
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
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */

  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  waitingRoom = xQueueCreate(100, sizeof(int));
  if(waitingRoom == 0)
  {
	  printf("Unable to build waiting room\n\r");
	  exit(1);
  }

  doorKey = xSemaphoreCreateMutex();
  if(doorKey == NULL)
  {
	  printf("Key to the door was lost. Cannot open today\n\r");
	  exit(1);
  }
  speakingStick = xSemaphoreCreateBinary();
  if(speakingStick == NULL)
  {
	  printf("No one showed up for work today\n\r");
	  exit(1);
  }
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */

  /* USER CODE BEGIN RTOS_THREADS */
  xTaskCreate(mng_Task, "Manager", 128, NULL, PriorityHigh, &h_mngTask);
  xTaskCreate(teller_Task, "Teller1", 128,(int *) 1, PriorityNormal, &h_teller1);
  xTaskCreate(teller_Task, "Teller1", 128,(int *) 2, PriorityNormal, &h_teller1);
  xTaskCreate(teller_Task, "Teller3", 128,(int *) 3, PriorityNormal, &h_teller3);
  xTaskCreate(uart_Task, "uart", 128, NULL, PriorityNormal, &h_uart);
  xTaskCreate(spinner_Task, "Spinning", 128, NULL, PriorityIdle, &h_spinner);

  /* Start scheduler */

  vTaskStartScheduler();

  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

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
  HAL_GPIO_WritePin(GPIOA, SHLD_D13_Pin|SHLD_D12_Pin|SHLD_D11_Pin|SHLD_D7_SEG7_Clock_Pin
                          |SHLD_D8_SEG7_Data_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SHLD_D3_Pin|SHLD_D4_SEG7_Latch_Pin|SHLD_D10_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

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

  /*Configure GPIO pins : SHLD_A1_Pin SHLD_A2_Pin */
  GPIO_InitStruct.Pin = SHLD_A1_Pin|SHLD_A2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SHLD_D13_Pin SHLD_D12_Pin SHLD_D11_Pin SHLD_D7_SEG7_Clock_Pin */
  GPIO_InitStruct.Pin = SHLD_D13_Pin|SHLD_D12_Pin|SHLD_D11_Pin|SHLD_D7_SEG7_Clock_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SHLD_A3_Pin */
  GPIO_InitStruct.Pin = SHLD_A3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SHLD_A3_GPIO_Port, &GPIO_InitStruct);

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

/* USER CODE BEGIN 4 */
void mng_Task( void * pvParameters )
{
	int cust_clk_S = 0;
	int cust_delay;
	TickType_t doorClosed = 0;
	TickType_t cust_delay_T;

	while(1)
	{
		cust_delay = num_gen(mngr);
		cust_delay_T = pdMS_TO_TICKS(cust_delay/0.6);

		vTaskDelayUntil(&doorClosed, cust_delay);
		cust_clk_S = timer * 0.6;
		doorClosed = xTaskGetTickCount();

		if(xQueueSendToBack(waitingRoom, &cust_clk_S, portMAX_DELAY) != pdPASS)
		{
			bad_QPost = 1;
		}

		else
		{
			//bad_QPost = 0;
			cust_cnt++;
		}


		if(timer >= 42000)
		{
			cust_clk_S = 50000;
			xQueueSendToBack(waitingRoom, &cust_clk_S, 5);
			//xSemaphoreTake(doorKey, portMAX_DELAY);
			vTaskDelete(h_mngTask);

		}

	}
}

void teller_Task( void * pvParameters )
{
	int cust_timeStamp_S;
	int tellerNum = (int) pvParameters;
	int greeting_time_S = 0;
	int process_delay;
	uint32_t randNum = 0;
	TickType_t greeting;
	TickType_t process_delay_T;

	teller_t currTeller;

	switch(tellerNum)
	{
	case 1:
		currTeller = tell1;
		break;
	case 2:
		currTeller = tell2;
		break;
	case 3:
		currTeller = tell3;
		break;
	}


	while(1)
	{
		if(xQueueReceive(waitingRoom, &cust_timeStamp_S, portMAX_DELAY) != pdPASS)
		{
			bad_QRead = 1;
		}
		else if(cust_timeStamp_S == 50000 && uxQueueMessagesWaiting(waitingRoom) == 0)
		{
			switch(tellerNum)
			{
			case 1:
				vTaskDelete(h_teller1);
				break;
			case 2:
				vTaskDelete(h_teller2);
				break;
			case 3:
				vTaskDelete(h_teller3);
				break;
			}
		}
		else
		{
			//bad_QRead = 0;
			currTeller.greeting_time = xTaskGetTickCount();
			greeting_time_S = timer * 0.6;
			wait_times[cust_cnt] = greeting_time_S - cust_timeStamp_S;
			process_delay = num_gen(process);
			help_times[cust_cnt] = process_delay;
			process_delay_T = pdMS_TO_TICKS(process_delay/0.6);
			vTaskDelayUntil(&greeting, process_delay_T);
			currTeller.cust_helped++;
		}



	}
}

void uart_Task( void * pvParameters )
{
	int hr = 0;
	int min = 0;
	int currTime_R = timer;
	int currTime_S = currTime_R * 0.6;

	TickType_t lastPrint;

	while(1)
	{
		currTime_R = timer;
		currTime_S = currTime_R * 0.6;

		if(currTime_R < 42000)
		{
			format_time(currTime_S, &hr, &min);
			if(min < 10)
			{
				printf("Current Time of Day: %d:0%d\n\r", hr, min);
			}
			else
			{
				printf("Current Time of Day: %d:%d\n\r", hr, min);
			}

			printf("Customers so far: %d\n\r", cust_cnt);
			printf("Number of customers helped: %d\n\n\n\r", cust_helped);

			/*if(eTaskGetState(h_teller1) == eSuspended)
			{
				printf("Teller 1 is busy\n\r");
			}
			else if(eTaskGetState(h_teller2) == eSuspended)
			{
				printf("Teller 2 is busy\n\r");
			}
			else if(eTaskGetState(h_teller2) == eSuspended)
			{
				printf("Teller 3 is busy\n\r");
			}*/

		}
		else if(bad_QPost)
		{
			printf("Customer Can't Get In!!!\n\r");
			bad_QPost = 0;
		}
		else if(bad_QRead)
		{
			printf("No customer for teller\n\r");
			bad_QRead = 0;
		}
		else if(currTime_R >= 42000 && currTime_R <= 43000)
		{
			printf("The day has ended\n\n\r");

			vTaskDelete(h_uart);
		}

		lastPrint = xTaskGetTickCount();
		vTaskDelayUntil(&lastPrint, pdMS_TO_TICKS(1000));
	}

}
void spinner_Task( void * pvParameters )
{
	TickType_t sixty_ticks = pdMS_TO_TICKS(60);

	  for(;;)
	  {
		  TickType_t prev = xTaskGetTickCount();
		  unsigned long cust_cnt = uxQueueMessagesWaiting(waitingRoom);
		  dig_ret(cust_cnt, dig_buffer);

		  WriteNumberToSegment(1, dig_buffer[0]);
		  WriteNumberToSegment(2, dig_buffer[1]);
		  WriteNumberToSegment(3, dig_buffer[2]);
		  WriteNumberToSegment(4, dig_buffer[3]);
		  vTaskDelayUntil(&prev, sixty_ticks);
	  }
}

/*void send(char *msg)
{
	xSemaphoreTake(speakingStick, 5);
	printf(msg);
	xSemaphoreGive(speakingStick);
}*/

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
	timer++;
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

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
	  printf("In Error Handler\n");
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
