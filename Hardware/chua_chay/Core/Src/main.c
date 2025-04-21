/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SAMPLE_NUMBER 30 * 40
#define DC_WINDOW 40
#define ENV_WINDOW 20
#define RS 0.5
#define RD 0.8

#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define BUTTON_Pin GPIO_PIN_4
#define BUTTON_GPIO_Port GPIOA
#define AIR_VALVE_Pin GPIO_PIN_5
#define AIR_VALVE_GPIO_Port GPIOA
#define CLK_MPS_Pin GPIO_PIN_8
#define CLK_MPS_GPIO_Port GPIOA
#define DOUT_MPS_Pin GPIO_PIN_11
#define DOUT_MPS_GPIO_Port GPIOA
#define PUMP_Pin GPIO_PIN_12
#define PUMP_GPIO_Port GPIOA
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
volatile uint8_t button_pressed = 0;
float sample_list[SAMPLE_NUMBER];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
uint32_t HX710B_ReadData(void);
float ConvertToPressure(uint32_t);
void control_pump(void);
void delay_us(uint32_t);
void uart_print(const char *);
int Init_HX710B(void);
uint32_t Get_Pressure(void);
void air_pump(uint32_t);
void exhaust(void);
void stop_exhaust(void);
void exhaust_all(void);
void start_sampling(void);
void Pressure_Calibration(void);
void sample(void);
void end_sample(void);
void process_data(float *data, uint16_t size, float *systolic, float *diastolic);
void blood_pressure_measurement(void);
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
  uint32_t sensor_value = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */
  Init_HX710B();
  uint32_t raw_adc = 0;
  float pressure_kpa = 0.0f;
  char *str_pressure;
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
  stop_exhaust();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // uint32_t raw_adc = Get_Pressure();
    // HAL_UART_Transmit(&huart1, (uint8_t *)&raw_adc, sizeof(raw_adc), HAL_MAX_DELAY);
    // HAL_Delay(500);
    if (button_pressed == 1)
    {
      Pressure_Calibration();
      // blood_pressure_measurement(); // Uncomment if full measurement is needed
      button_pressed = 0;
    }
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

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1599; // For 16MHz clock, PWM freq = 10Hz
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim2);
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
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
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOA, CLK_MPS_Pin | PUMP_Pin, GPIO_PIN_RESET);

  /* Configure LED_Pin (PC13) */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /* Configure BUTTON_Pin (PA4) with interrupt */
  GPIO_InitStruct.Pin = BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BUTTON_GPIO_Port, &GPIO_InitStruct);

  /* Configure CLK_MPS_Pin (PA8) for HX710B clock */
  GPIO_InitStruct.Pin = CLK_MPS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(CLK_MPS_GPIO_Port, &GPIO_InitStruct);

  /* Configure DOUT_MPS_Pin (PA11) for HX710B data */
  GPIO_InitStruct.Pin = DOUT_MPS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DOUT_MPS_GPIO_Port, &GPIO_InitStruct);

  /* Configure PUMP_Pin (PA12) */
  GPIO_InitStruct.Pin = PUMP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PUMP_GPIO_Port, &GPIO_InitStruct);

  /* Enable NVIC for EXTI4 (BUTTON_Pin on PA4) */
  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);
}

/* USER CODE BEGIN 4 */

/**
  * @brief  Delays execution for a specified number of microseconds.
  * @param  us: Number of microseconds to delay.
  * @retval None
  * @note   Calibrated for 16MHz system clock.
  */
void delay_us(uint32_t us)
{
  uint32_t cycles_per_us = 6;
  uint32_t i;
  for (i = 0; i < (us * cycles_per_us); i++)
  {
    __NOP();
  }
}

/**
  * @brief  Controls air pump based on target pressure.
  * @param  pressure: Target pressure in mmHg.
  * @retval None
  */
void air_pump(uint32_t pressure)
{
  uint32_t pressure_current = 0;
  uint8_t lp = 1;

  HAL_GPIO_WritePin(PUMP_GPIO_Port, PUMP_Pin, GPIO_PIN_SET);
  while (lp)
  {
    pressure_current = Get_Pressure();
    if (pressure_current >= pressure)
    {
      HAL_GPIO_WritePin(PUMP_GPIO_Port, PUMP_Pin, GPIO_PIN_RESET);
      lp = 0;
    }
  }
}

/**
  * @brief  Initializes HX710B sensor.
  * @retval int: 0 on success, -1 on timeout.
  */
int Init_HX710B(void)
{
  uint8_t i;
  HAL_GPIO_WritePin(CLK_MPS_GPIO_Port, CLK_MPS_Pin, GPIO_PIN_RESET);
  uint32_t timeout = 100000;
  while (HAL_GPIO_ReadPin(DOUT_MPS_GPIO_Port, DOUT_MPS_Pin) == GPIO_PIN_SET && timeout--)
  {
    delay_us(1);
  }
  if (timeout == 0) return -1;

  for (i = 0; i < 27; i++)
  {
    HAL_GPIO_WritePin(CLK_MPS_GPIO_Port, CLK_MPS_Pin, GPIO_PIN_SET);
    delay_us(1);
    HAL_GPIO_WritePin(CLK_MPS_GPIO_Port, CLK_MPS_Pin, GPIO_PIN_RESET);
    delay_us(1);
  }
  return 0;
}

/**
  * @brief  Reads data from HX710B sensor.
  * @retval uint32_t: 24-bit ADC value, -1 on timeout.
  */
uint32_t HX710B_ReadData(void)
{
  uint32_t data = 0;
  uint8_t i;

  HAL_GPIO_WritePin(CLK_MPS_GPIO_Port, CLK_MPS_Pin, GPIO_PIN_RESET);
  uint32_t timeout = 100000;
  while (HAL_GPIO_ReadPin(DOUT_MPS_GPIO_Port, DOUT_MPS_Pin) == GPIO_PIN_SET && timeout--)
  {
    delay_us(1);
  }
  if (timeout == 0) return -1;

  for (i = 0; i < 24; i++)
  {
    HAL_GPIO_WritePin(CLK_MPS_GPIO_Port, CLK_MPS_Pin, GPIO_PIN_SET);
    delay_us(1);
    data = (data << 1) | HAL_GPIO_ReadPin(DOUT_MPS_GPIO_Port, DOUT_MPS_Pin);
    HAL_GPIO_WritePin(CLK_MPS_GPIO_Port, CLK_MPS_Pin, GPIO_PIN_RESET);
    delay_us(1);
  }
  return data;
}

/**
  * @brief  Retrieves pressure from HX710B sensor.
  * @retval uint32_t: Pressure in mmHg.
  */
uint32_t Get_Pressure(void)
{
  uint32_t pressure = HX710B_ReadData();
  pressure = (uint32_t)ConvertToPressure(pressure);
  return pressure;
}

/**
  * @brief  Converts ADC value to pressure in mmHg.
  * @param  adc_value: 24-bit ADC value.
  * @retval float: Pressure in mmHg.
  */
float ConvertToPressure(uint32_t adc_value)
{
  int32_t signed_adc = (int32_t)(adc_value & 0xFFFFFF);
  if (signed_adc >= 0x800000)
  {
    signed_adc -= 0x1000000;
  }
  const float offset = 1359514.0;
  const float adc_max = 8388607.0;
  const float pressure_max = 300.0;
  static const float slope = pressure_max / (adc_max - offset);
  float pressure = (signed_adc - offset) * slope;
  return (pressure < 0.0) ? 0.0 : pressure;
}

/**
  * @brief  Prints string via UART.
  * @param  str: String to print.
  * @retval None
  */
void uart_print(const char *str)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

/**
  * @brief  EXTI4 Interrupt handler for BUTTON_Pin (PA4).
  * @retval None
  */
void EXTI4_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(BUTTON_Pin);
}

/**
  * @brief  GPIO EXTI callback.
  * @param  GPIO_Pin: Pin that triggered interrupt.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  static uint32_t last_time = 0;
  uint32_t current_time = HAL_GetTick();
  if (GPIO_Pin == BUTTON_Pin && (current_time - last_time > 20))
  {
    button_pressed = 1;
    last_time = current_time;
  }
}

/**
  * @brief  Toggles pump GPIO pin.
  * @retval None
  */
void control_pump(void)
{
  HAL_GPIO_TogglePin(PUMP_GPIO_Port, PUMP_Pin);
}

/**
  * @brief  Sets PWM to exhaust air (20% duty).
  * @retval None
  */
void exhaust(void)
{
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 200); // 20% of 999
}

/**
  * @brief  Stops exhaust (0% duty).
  * @retval None
  */
void stop_exhaust(void)
{
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
}

/**
  * @brief  Fully exhausts air (100% duty).
  * @retval None
  */
void exhaust_all(void)
{
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 999);
}

/**
  * @brief  Starts sampling process.
  * @retval None
  */
void start_sampling(void)
{
  stop_exhaust();
  air_pump(180);
  delay_us(500);
}

/**
  * @brief  Samples pressure data.
  * @retval None
  */
void sample(void)
{
  uint8_t i = 0;
  uint32_t data = 0;
  for (i = 0; i < SAMPLE_NUMBER; i++)
  {
    data = Get_Pressure();
    sample_list[i] = data;
  }
}

/**
  * @brief  Ends sampling and processes data.
  * @retval None
  */
void end_sample(void)
{
  float systolic = 0, diastolic = 0;
  char buffer[50];
  exhaust_all();
  process_data(sample_list, SAMPLE_NUMBER, &systolic, &diastolic);
  sprintf(buffer, "Systolic: %.1f mmHg, Diastolic: %.1f mmHg\r\n", systolic, diastolic);
  HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
}

/**
  * @brief  Measures blood pressure.
  * @retval None
  */
void blood_pressure_measurement(void)
{
  start_sampling();
  sample();
  end_sample();
}

/**
  * @brief  Calibrates pressure sensor.
  * @retval None
  */
void Pressure_Calibration(void)
{
  uint8_t lp = 1;
  uint32_t pressure = 0;

  stop_exhaust();
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
  air_pump(180);
  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
  HAL_Delay(5000);

  while (lp)
  {
    HAL_Delay(1000 / 40);
    pressure = Get_Pressure();
    if (pressure < 170)
    {
      HAL_UART_Transmit(&huart1, (uint8_t *)&pressure, sizeof(pressure), HAL_MAX_DELAY);
      HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
      HAL_Delay(500);
    }
    else
    {
      HAL_UART_Transmit(&huart1, (uint8_t *)&pressure, sizeof(pressure), HAL_MAX_DELAY);
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
      exhaust_all();
      lp = 0;
    }
  }
}

/**
  * @brief  Processes data to calculate systolic and diastolic pressures.
  * @param  data: Input data array.
  * @param  size: Size of data array.
  * @param  systolic: Pointer to systolic value.
  * @param  diastolic: Pointer to diastolic value.
  * @retval None
  */
void process_data(float *data, uint16_t size, float *systolic, float *diastolic)
{
  float dc[size], ac[size], envelope[size];

  for (int i = 0; i < size; i++)
  {
    float sum = 0;
    int count = 0;
    for (int j = i - DC_WINDOW / 2; j < i + DC_WINDOW / 2; j++)
    {
      if (j >= 0 && j < size)
      {
        sum += data[j];
        count++;
      }
    }
    dc[i] = sum / count;
    ac[i] = data[i] - dc[i];
  }

  for (int i = 0; i < size; i++)
  {
    float sum = 0;
    int count = 0;
    for (int j = i - ENV_WINDOW / 2; j < i + ENV_WINDOW / 2; j++)
    {
      if (j >= 0 && j < size)
      {
        sum += fabs(ac[j]);
        count++;
      }
    }
    envelope[i] = sum / count;
  }

  int max_index = 0;
  for (int i = 1; i < size; i++)
  {
    if (envelope[i] > envelope[max_index])
      max_index = i;
  }
  float max_envelope = envelope[max_index];

  int systolic_index = max_index;
  while (systolic_index > 0 && envelope[systolic_index] >= RS * max_envelope)
  {
    systolic_index--;
  }

  int diastolic_index = max_index;
  while (diastolic_index < size - 1 && envelope[diastolic_index] >= RD * max_envelope)
  {
    diastolic_index++;
  }

  *systolic = dc[systolic_index];
  *diastolic = dc[diastolic_index];
}

/* USER CODE END 4 */

/**
  * @brief  Error handler.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports assert errors.
  * @param  file: Source file name.
  * @param  line: Line number.
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */