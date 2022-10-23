/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 563 EXAM
  ******************************************************************************
  * @attention
  *
  *  Please Read all instructions and only put your code in the areas provided
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdlib.h"
#include "ctype.h"
#include "stdio.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



// function prototypes for programming examination functions -- do not change
void function_one( void ) ;
void function_two( void ) ;
void function_three( void ) ;

// Checks for Button key presses
// Calls function_one when the S1 button is pressed.
// Calls function_two when the S2 button is pressed.
//
// DO NOT CHANGE this function.
void check_btns()
{

	// Start Counter DISPLAY
	if (HAL_GPIO_ReadPin(SHLD_A1_GPIO_Port, SHLD_A1_Pin)==GPIO_PIN_RESET) {
		function_one();																	//your function
		while(HAL_GPIO_ReadPin(SHLD_A1_GPIO_Port, SHLD_A1_Pin)==GPIO_PIN_RESET);
	}

	// Stop counter DISPLAY
	if(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET){
		function_two() ;																//your function
		while(HAL_GPIO_ReadPin(SHLD_A2_GPIO_Port, SHLD_A2_Pin)==GPIO_PIN_RESET);
	}

}

// Returns the current count value from timer 2.
// DO NOT CHANGE this function.
uint32_t get_timer2_count()
{
	return TIM2->CNT ;
}


// Turns on  LED if "on" parameter is non-zero, turns it DIM if zero.
// Note inverted logic as pull down (low) activates LED
// DO NOT CHANGE this function.
void set_led_1( uint32_t on )
{
	if ( on ) // a5
		HAL_GPIO_WritePin(SHLD_D13_GPIO_Port, SHLD_D13_Pin, GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(SHLD_D13_GPIO_Port, SHLD_D13_Pin, GPIO_PIN_SET);
}

void set_led_2( uint32_t on )
{
	if ( on ) //a6
		HAL_GPIO_WritePin(SHLD_D12_GPIO_Port, SHLD_D12_Pin, GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(SHLD_D12_GPIO_Port, SHLD_D12_Pin, GPIO_PIN_SET);
}

void set_led_3( uint32_t on )
{
	if ( on ) //a7
		HAL_GPIO_WritePin(SHLD_D11_GPIO_Port, SHLD_D11_Pin, GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(SHLD_D11_GPIO_Port, SHLD_D11_Pin, GPIO_PIN_SET);
}

void set_led_4( uint32_t on )
{
	if ( on )
		HAL_GPIO_WritePin(SHLD_D10_GPIO_Port, SHLD_D10_Pin, GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(SHLD_D10_GPIO_Port, SHLD_D10_Pin, GPIO_PIN_SET);
}


////////////////////////// 7-segment stuff /////////////////////////////////////////////

/// Do nothing HERE!
void shiftOut(GPIO_TypeDef* data_port, uint16_t data_pin, GPIO_TypeDef* clock_port, uint16_t clock_pin, uint8_t value) {
	for(int ii=0x80; ii; ii>>=1) {
		HAL_GPIO_WritePin(clock_port, clock_pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(data_port, data_pin, (value&ii)!=0);
		HAL_GPIO_WritePin(clock_port, clock_pin, GPIO_PIN_SET);
	}
}

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
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim2);						// Starts the timer for you!

  // Clears the 7 Seg Display
  HAL_GPIO_WritePin(SHLD_D4_SEG7_Latch_GPIO_Port, SHLD_D4_SEG7_Latch_Pin, GPIO_PIN_RESET);
  shiftOut(SHLD_D8_SEG7_Data_GPIO_Port, SHLD_D8_SEG7_Data_Pin, SHLD_D7_SEG7_Clock_GPIO_Port, SHLD_D7_SEG7_Clock_Pin, 0);
  shiftOut(SHLD_D8_SEG7_Data_GPIO_Port, SHLD_D8_SEG7_Data_Pin, SHLD_D7_SEG7_Clock_GPIO_Port, SHLD_D7_SEG7_Clock_Pin, 0);
  HAL_GPIO_WritePin(SHLD_D4_SEG7_Latch_GPIO_Port, SHLD_D4_SEG7_Latch_Pin, GPIO_PIN_SET);	// latch data

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  /* DO NOT MODIFY ANYTHING HERE this loop must run continuously for your program to work */
  while (1)
  {
	  check_btns();				  /* DO NOT MODIFY ANYTHING HERE */
	  function_three();
	  HAL_GPIO_WritePin(SHLD_D10_GPIO_Port, SHLD_D10_Pin, !(__HAL_TIM_GET_COUNTER(&htim2)& 0x3c0));
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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 16;
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
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
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
  htim2.Init.Prescaler = 15999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
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
  HAL_GPIO_WritePin(GPIOB, SHLD_D4_SEG7_Latch_Pin|SHLD_D10_Pin, GPIO_PIN_SET);

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

  /*Configure GPIO pins : USART_TX_Pin USART_RX_Pin */
  GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
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


///////////////////////////////////////////////////////////////////////////////
// Implement the three functions described below. You are allowed to         //
// add data declarations outside the functions if you find that helpful.     //
// Only enter your code below this comment block. Do not change any code     //
// above this comment block. However, you can call any of the functions      //
// that are provided above. Or any HAL routines you might find helpful.      //
///////////////////////////////////////////////////////////////////////////////

uint32_t is_running = 0 ;		// boolean flag to control whether or not we are running.

// This function is called when the S1-A1 button is pressed.
//
// The purpose of this function is to enable the counter display
// in function_three. An appropriate name for this function would be "start".
// Tip -- Do NOT change the LEDs in this function. Your code in this function
//        should be very simple!
void function_one()
{
	is_running = 1;
}

// This function is called when the S2-A2 button is pressed.
//
// The purpose of this function is to disable the counter display
// in function_three. An appropriate name for this function would be "stop".
// Tip -- Do NOT change the LEDs in this function. Your code in this function
//        should be very simple!
void function_two()
{
	is_running = 0;
}

// This function is called from the infinite loop in main
// right after the buttons are checked.
//
// This function must implement a simple 3 bit counter display using the D1,D2,D3 LEDs
// that changes state every 1.024 seconds. This interval was chosen to simplify your code!
// NOTE -- This 3 bit BINARY COUNTER only runs when it is enabled by function_one.
//         It stops when disabled by function_two.
//
// The D1 LED turns dim for exactly 1.024 seconds and on for exactly 1.024 seconds. This represents bit 0.
// The D2 LED turns dim for exactly 2.048 seconds and on for exactly 2.048 seconds. This represents bit 1.
// The D3 LED turns dim for exactly 4.096 seconds and on for exactly 4.096 seconds. This represents bit 2.

// For full credit these LEDs must be turn on and off exactly as described in this 8.192 second cycle:
// 1. at 0.000 seconds -- the D1 LED is    dim; the D2 LED is    dim; the D3 LED is    dim;
// 2. at 1.024 seconds -- the D1 LED turns on ; the D2 LED stays dim; the D3 LED stays dim;
// 3. at 2.048 seconds -- the D1 LED turns dim; the D2 LED turns on ; the D3 LED stays dim;
// 4. at 3.072 seconds -- the D1 LED turns on ; the D2 LED stays on ; the D3 LED stays dim;
// 5. at 4.096 seconds -- the D1 LED turns dim; the D2 LED turns dim; the D3 LED turns on ;
// 6. at 5.120 seconds -- the D1 LED turns on ; the D2 LED stays dim; the D3 LED stays on ;
// 7. at 6.144 seconds -- the D1 LED turns dim; the D2 LED turns on ; the D3 LED stays on ;
// 8. at 7.168 seconds -- the D1 LED turns on ; the D2 LED turns on ; the D3 LED stays on ;
// 9. at 8.192 seconds the cycle starts at step (1) 0.0000 seconds.
// Keep repeating this cycle forever.
//
// NOTE -- The LED's show a low level illumination (DIM) while off and are very bright while on.
// NOTE -- your code will be inspected to confirm this exact timing.


void function_three()
{
	uint32_t off = 0;

	if(is_running == 1)
	{
		uint32_t count = get_timer2_count();

		switch (count)
		{
			case 0:
				set_led_1(0);
				set_led_2(0);
				set_led_3(0);
				set_led_4(0);
				break;
			case 1024:
				set_led_1(1);
				break;
			case 2048:
				set_led_1(0);
				set_led_2(1);
				break;
			case 3072:
				set_led_1(1);
				set_led_2(1);
				break;
			case 4096:
				set_led_1(0);
				set_led_2(0);
				set_led_3(1);
				break;
			case 5120:
				set_led_1(1);
				set_led_2(0);
				set_led_3(1);
				break;
			case 6144:
				set_led_1(0);
				set_led_2(1);
				break;
			case 7168:
				set_led_1(1);
				set_led_2(1);
		}
		if(count > 8192)
		{
			TIM2->CNT = 0;
		}
	}

}


/* USER CODE END 4 */

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
