/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @atdisption
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
	int break_start;
	int break_cnt;
	int done_time;
	int cust_timer;
	int cust_helped;
	int break_times[20];

}teller_t;


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
int line_len = 0;
int t1 = 1;
int t2 = 2;
int t3 = 3;
int spinner = 0;
int help_times[200] = {0};
int wait_times[200] = {0};
_Bool bad_QPost = 0;
_Bool bad_QRead = 0;

teller_t tell1;
teller_t tell2;
teller_t tell3;

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
  xTaskCreate(mng_Task, "Manager", 256, NULL, PriorityHigh, &h_mngTask);
  xTaskCreate(teller_Task, "Teller1", 256, (int *) 1, PriorityNormal, &h_teller1);
  xTaskCreate(teller_Task, "Teller1", 256, (int *) 2, PriorityNormal, &h_teller1);
  xTaskCreate(teller_Task, "Teller3", 256, (int *) 3, PriorityNormal, &h_teller3);
  xTaskCreate(uart_Task, "uart", 256, NULL, PriorityNormal, &h_uart);
  xTaskCreate(spinner_Task, "Spinning", 128, NULL, PriorityLow, &h_spinner);

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
	int new_break_time = 0;
	TickType_t cust_clk_T = 0;
	TickType_t cust_delay_T;

	while(1)
	{
		cust_delay = num_gen(mngr); //Generate time until new customer enters
		cust_delay_T = pdMS_TO_TICKS(cust_delay/0.6); //Convert to ticks

		vTaskDelayUntil(&cust_clk_T, cust_delay_T); // Suspend until new customer enters
		cust_clk_S = timer * 0.6; //Grab and convert customer entrance to simulated time
		cust_clk_T = xTaskGetTickCount(); //Update latest customer entrance in ticks

		if(xQueueSendToBack(waitingRoom, &cust_clk_S, portMAX_DELAY) != pdPASS)
		{
			bad_QPost = 1;
		}

		else
		{
			bad_QPost = 0;
			cust_cnt++;
			int currLen = uxQueueMessagesWaiting(waitingRoom);
			if(currLen > line_len)
			{
				line_len = currLen;
			}
		}

		if(timer >= 42000) //If end of the day
		{
			cust_clk_S = 50000; //Signal tellers they can go home
			xQueueSendToBack(waitingRoom, &cust_clk_S, portMAX_DELAY);
			//xSemaphoreTake(doorKey, portMAX_DELAY);
			vTaskDelete(h_mngTask); //Manager goes home

		}

	}
}

void teller_Task( void * pvParameters )
{
	int cust_timeStamp_S;
	int tellerNum = (int) pvParameters;
	int greeting_time_S = 0;
	int process_delay;
	int new_break_start;
	int break_dur;

	int currTime = 0;
	int i1 = 0;
	int i2 = 0;
	int i3 = 0;
	_Bool b_flg = 0;

	TickType_t break_start_T;
	TickType_t process_delay_T;
	TickType_t break_dur_T;
	TaskHandle_t currHandle;

	teller_t *currTeller = NULL;

	switch(tellerNum) //Assign currect struct to the current task
	{
	case 1:
		currTeller = &tell1;
		break;
	case 2:
		currTeller = &tell2;
		break;
	case 3:
		currTeller = &tell3;
		break;
	}


	while(1)
	{
		if(!b_flg)
		{
			new_break_start = num_gen(break_start); //Generate how long to wait until next break
			b_flg = 1;
		}

		/* Check buttons to see if any tellers need to be put on break */
		while(HAL_GPIO_ReadPin(SHLD_A1_GPIO_Port, SHLD_A1_Pin)==GPIO_PIN_RESET)
		{
			vTaskSuspend(h_teller1);
			tell1.break_start = timer * 0.6;
			tell1.break_cnt++;

			if(HAL_GPIO_ReadPin(SHLD_A1_GPIO_Port, SHLD_A1_Pin)==GPIO_PIN_SET)
			{
				currTime = timer * 0.6;
				tell1.break_times[i1] = currTime - tell1.break_start;
				i1++;
				vTaskResume(h_teller1);

			}
		}

		while(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET)
		{
			vTaskSuspend(h_teller2);
			tell2.break_start = timer * 0.6;
			tell2.break_cnt++;

			if(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_SET)
			{
				currTime = timer * 0.6;
				tell2.break_times[i2] = currTime - tell2.break_start;
				i2++;
				vTaskResume(h_teller2);
			}
		}

		while(HAL_GPIO_ReadPin(SHLD_A3_GPIO_Port, SHLD_A3_Pin)==GPIO_PIN_RESET)
		{
			vTaskSuspend(h_teller3);
			tell3.break_start = timer * 0.6;
			tell3.break_cnt++;

			if(HAL_GPIO_ReadPin(SHLD_A3_GPIO_Port, SHLD_A3_Pin)==GPIO_PIN_SET)
			{
				currTime = timer * 0.6;
				tell3.break_times[i3] = currTime - tell3.break_start;
				i3++;
				vTaskResume(h_teller3);
			}
		}
		if(timer <= (new_break_start + currTeller->break_start + 20)/0.6 && timer >= (new_break_start + currTeller->break_start)/0.6) //Break due to random break scheduling
		{
			b_flg = 0;
			currTeller->break_cnt++; //Increment teller break count
			currTeller->break_start = timer * 0.6; //Update break time. In simulated time

			break_dur = num_gen(break_len); //Generated random break length

			/*Convert new break start value and break duration to tick values before suspending task for full duration*/
			break_start_T = pdMS_TO_TICKS(currTeller->break_start);
			break_dur_T = pdMS_TO_TICKS(break_dur/0.6);

			vTaskDelayUntil(&break_start_T, break_dur_T);

		}
		/*If no break then perform teller duties */
		if(xQueueReceive(waitingRoom, &cust_timeStamp_S, portMAX_DELAY) != pdPASS) //Check if reading from Queue was successful
		{
			bad_QRead = 1;
		}
		else if(cust_timeStamp_S == 50000 && uxQueueMessagesWaiting(waitingRoom) == 0) //If day ended and line empty tellers get to go home
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
			currHandle = xTaskGetCurrentTaskHandle();
			vTaskDelete(currHandle);
		}
		else //Normal function
		{
			//bad_QRead = 0;
			currTeller->greeting_time = xTaskGetTickCount(); //Start of customer processing
			greeting_time_S = timer * 0.6; //Grab simulated time of day that customer processing began
			wait_times[cust_cnt] = greeting_time_S - cust_timeStamp_S; //Save the amount of time the cust waited in line
			process_delay = num_gen(process); //Generate duration of customer processing
			help_times[cust_cnt] = process_delay; //Save process duration
			process_delay_T = pdMS_TO_TICKS(process_delay/0.6); //Convert to ticks
			vTaskDelayUntil(&currTeller->greeting_time, process_delay_T); //Suspend
			currTeller->cust_helped++; //Customer helped
		}



	}
}

/* Task to handle all printing functions so there is never conflict over shared resource*/
void uart_Task( void * pvParameters )
{
	int hr = 0;
	int min = 0;
	int currTime_R = timer;
	int currTime_S = currTime_R * 0.6;


	TickType_t lastPrint;

	stats_t iQueue;
	stats_t wTeller;
	stats_t t1Breaks;
	stats_t t2Breaks;
	stats_t t3Breaks;


	while(1)
	{
		currTime_R = timer; //Current real time
		currTime_S = currTime_R * 0.6; //Current simulated time
		int custsH = tell1.cust_helped + tell2.cust_helped + tell3.cust_helped;

		if(currTime_R < 42000) //Normal day operation
		{
			format_time(currTime_S, &hr, &min); //Current simulated time is in seconds since opening. Format time into 12-hr clock
			if(min < 10)
			{
				printf("Current Time of Day: %d:0%d\n\r", hr, min);
			}
			else
			{
				printf("Current Time of Day: %d:%d\n\r", hr, min);
			}

			printf("Customers so far: %d\n\r", cust_cnt);
			printf("Number of customers helped: %d\n\n\n\r", custsH);


		}
		else if(bad_QPost)//If manager couldn't put customer in queue
		{
			printf("Customer Can't Get In!!!\n\r");
			bad_QPost = 0;
		}
		else if(bad_QRead)//If teller couldn't pull customer from line
		{
			printf("No customer for teller\n\r");
			bad_QRead = 0;
		}
		else if(currTime_R >= 42000 && uxQueueMessagesWaiting(waitingRoom) == 0)//If end of day
		{
			iQueue = calcs(wait_times, 0);
			wTeller = calcs(help_times, 0);
			t1Breaks = calcs(tell1.break_times, 1);
			t2Breaks = calcs(tell2.break_times, 1);
			t3Breaks = calcs(tell3.break_times, 1);

			printf("\t--------- The day has ended ---------\n\n\r");
			printf("Total customers today: %d\n\r", cust_cnt);
			printf("\tAverage wait in line: %d Sec\tMax wait in line: %d Sec\n\r", iQueue.ave, iQueue.max);
			printf("\tAverage time with Teller: %d Sec\tMax time with Teller: %d Sec\n\r", wTeller.ave, wTeller.max);
			printf("Teller 1 helped: %d customers\n", tell1.cust_helped);
			printf("\tTook %d breaks. Longest: %d Sec\tAve duration: %d Sec\n\r", tell1.break_cnt, t1Breaks.max, t1Breaks.ave);
			printf("Teller 2 helped: %d customers\n", tell2.cust_helped);
			printf("\tTook %d breaks. Longest: %d Sec\tAve duration: %d Sec\n\r", tell2.break_cnt, t2Breaks.max, t2Breaks.ave);
			printf("Teller 3 helped: %d customers\n", tell3.cust_helped);
			printf("\tTook %d breaks. Longest: %d Sec\tAve duration: %d Sec\n\r", tell3.break_cnt, t3Breaks.max, t3Breaks.ave);
			printf("Max length of the line: %d\n\r", line_len);
			printf("Percent of Total time Spent in idle task: %d\n\n\r", spinner);
			vTaskDelete(h_uart);
		}

		/* Controls frequency of print statements */
		lastPrint = xTaskGetTickCount();
		vTaskDelayUntil(&lastPrint, pdMS_TO_TICKS(1000));
	}

}

/* Idle task in charge of posting number of customers in line to the 7-seg display */
void spinner_Task( void * pvParameters )
{
	TickType_t disp_ticks = pdMS_TO_TICKS(50);

	  for(;;)
	  {
		  TickType_t prev = xTaskGetTickCount();
		  unsigned long curr_cust_cnt = uxQueueMessagesWaiting(waitingRoom);

		  if(timer >= 42000)
		  {
			  vTaskDelete(h_spinner);
		  }

		  WriteNumberToSegment(3, curr_cust_cnt);
		  vTaskDelayUntil(&prev, disp_ticks);
		  //prev = xTaskGetTickCount();
/*		  WriteNumberToSegment(2, dig_buffer[1]);
		  vTaskDelayUntil(&prev, disp_ticks);
		  prev = xTaskGetTickCount();
		  WriteNumberToSegment(1, dig_buffer[2]);
		  vTaskDelayUntil(&prev, disp_ticks);
		  prev = xTaskGetTickCount();
		  WriteNumberToSegment(0, dig_buffer[3]);
		  vTaskDelayUntil(&prev, disp_ticks);*/
	  }
}

void vApplicationIdleHook(void)
{
	spinner++;
}

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
