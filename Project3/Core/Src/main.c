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

#include "time_calc.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct teller{
	TickType_t greeting_time;
	int done_time;
	int cust_timer;

}teller_t;

teller_t helpDesk[3];

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_DELAY 65535
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RNG_HandleTypeDef hrng;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

osThreadId spinnerTaskHandle;
/* USER CODE BEGIN PV */

QueueHandle_t waitingRoom;
SemaphoreHandle_t doorKey;
SemaphoreHandle_t speakingStick;

TaskHandle_t h_mngTask;
TaskHandle_t h_teller1;
TaskHandle_t h_teller2;
TaskHandle_t h_teller3;
TaskHandle_t h_uart;
//TaskHandle_t h_spinner;

int cust_cnt = 0;
int cust_helped = 0;
int timer = 0;
_Bool bad_QPost = 0;
_Bool bad_QRead = 0;

teller_t tell1;
teller_t tell2;
teller_t tell3;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_RNG_Init(void);
static void MX_TIM2_Init(void);

/* USER CODE BEGIN PFP */
void mng_Task( void * pvParameters );
void teller_Task( void * pvParameters );
void uart_Task( void * pvParameters );
//void spinner_Task( void * pvParameters );
void send(char *msg);
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
  //xTaskCreate(spinner_Task, "Spinning", 128, NULL, PriorityIdle, &h_spinner);

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
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 20000-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

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

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void mng_Task( void * pvParameters )
{
	uint32_t randNum = 0;
	int cust_clk_ST = 0;
	TickType_t doorClosed = 0;
	TickType_t cust_delay;
	TIM2->CNT = 0;

	helpDesk[0] = tell1;
	helpDesk[1] = tell2;
	helpDesk[2] = tell3;

	HAL_TIM_Base_Start(&htim2);

	while(1)
	{
		HAL_RNG_GenerateRandomNumber(&hrng, &randNum);
		randNum = randNum & 0xFF;

		if(randNum > 240)
		{
			randNum = 240;
		}
		else if(randNum < 60)
		{
			randNum = 60;
		}

		cust_delay = pdMS_TO_TICKS(randNum/0.6);

		vTaskDelayUntil(&doorClosed, cust_delay);
		cust_clk_ST = timer * 0.6;
		doorClosed = xTaskGetTickCount();

		if(xQueueSendToBack(waitingRoom, &cust_clk_ST, portMAX_DELAY) != pdPASS)
		{
			bad_QPost = 1;
		}

		else
		{
			bad_QPost = 0;
			cust_cnt++;
		}


		/*if(TIM2->CNT >= 42000)
		{
			 Day has ended. Time to lock the bank door
			TIM2->CNT = 0;
			HAL_TIM_Base_Stop(&htim2);
			cust_clk_ST = 50000;
			xQueueSendToBack(waitingRoom, &cust_clk_ST, 5);
			//xSemaphoreTake(doorKey, portMAX_DELAY);
			sprintf(msg, "Day Has Ended. Bank Closing\n\r");
			//send(msg);
			//vTaskDelay(portMAX_DELAY);

		}*/

	}
}

void teller_Task( void * pvParameters )
{
	int cust_timeStamp_ST;
	int tellerNum = (int) pvParameters;
	int greeting_time_ST = 0;
	uint32_t randNum = 0;
	TickType_t greeting;
	TickType_t process_delay;


	while(1)
	{
		if(xQueueReceive(waitingRoom, &cust_timeStamp_ST, portMAX_DELAY) != pdPASS)
		{
			bad_QRead = 1;
		}
		else if(cust_timeStamp_ST == 50000)
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
			bad_QRead = 0;
			helpDesk[tellerNum].greeting_time = xTaskGetTickCount();
			greeting_time_ST = timer * 0.6;
			helpDesk[tellerNum].cust_timer = greeting_time_ST - cust_timeStamp_ST;

			HAL_RNG_GenerateRandomNumber(&hrng, &randNum);
			randNum = randNum & 0x1FF;

			if(randNum > 480)
			{
				randNum = 480;
			}
			else if(randNum < 30)
			{
				randNum = 30;
			}

			process_delay = pdMS_TO_TICKS(randNum/0.6);
			vTaskDelayUntil(&greeting, process_delay);
			cust_helped++;
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

			printf("Wall clock Time: %d\n\r", currTime_R);

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

			lastPrint = xTaskGetTickCount();


			vTaskDelayUntil(&lastPrint, pdMS_TO_TICKS(1000));
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
		else if(currTime_R >= 42000)
		{
			printf("The day has ended");

			vTaskDelete(h_uart);
		}
	}

}
/*void spinner_Task( void * pvParameters )
{
	  char msg[50];

	  for(;;)
	  {

		  sprintf(msg, "OS Spinning\n\r");
		  //send(msg);
		  vTaskDelay(pdMS_TO_TICKS(1000));
	  }
}*/

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
