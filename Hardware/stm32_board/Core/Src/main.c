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

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP alal */
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
volatile uint8_t button_pressed_k1 = 0;
#define SAMPLE_NUMBER 30 * 40
float sample_list[SAMPLE_NUMBER];
#define DC_WINDOW 40
#define ENV_WINDOW 20
#define RS 0.5
#define RD 0.8
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
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  Init_HX710B();
  // Khai báo biến toàn cục
  uint32_t raw_adc = 0;
  float pressure_kpa = 0.0f;
  char *str_pressure;
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_GPIO_WritePin(LED_D3_GPIO_Port, LED_D3_Pin, GPIO_PIN_SET);
  stop_exhaust();
  /* Infinite loop */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    if (button_pressed_k1 == 1)
    {
      // HAL_GPIO_TogglePin(LED_D3_GPIO_Port, LED_D3_Pin);
      // control_pump();
      Pressure_Calibration();
      // blood_pressure_measurement();

      button_pressed_k1 = 0;
    }
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
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
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 1599;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
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
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
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
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, HX710B_SCK_Pin | PUMP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : HX710B_SCK_Pin PUMP_Pin */
  GPIO_InitStruct.Pin = HX710B_SCK_Pin | PUMP_Pin | LED_D3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : HX710B_DT_Pin */
  GPIO_InitStruct.Pin = HX710B_DT_Pin | BUTTON_K1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(HX710B_DT_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  GPIO_InitStruct.Pin = BUTTON_K1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BUTTON_K1_GPIO_Port, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
 * @brief  Delays execution for a specified number of microseconds.
 * @param  us: Number of microseconds to delay.
 * @retval None
 * @note   This function assumes a system clock of 16MHz.
 *         Adjust cycles_per_us if the system clock is different.
 */
void delay_us(uint32_t us)
{
  // At 16MHz, 1μs corresponds to 16 cycles
  // A simple for loop takes about 3 cycles (depending on the compiler),
  // hence loop approximately 5-6 times for 1μs delay
  uint32_t cycles_per_us = 6; // Adjust based on actual timing
  uint32_t i;

  // Loop to create the delay
  for (i = 0; i < (us * cycles_per_us); i++)
  {
    __NOP(); // No Operation, used to consume clock cycles
  }
}

/**
 * @brief  Controls the air pump based on the given pressure.
 * @param  pressure: Target pressure value.
 * @retval None
 * @details
 *         This function controls the air pump by setting the GPIO pin high when
 *         the pressure is below the target value and low when it's above.
 *         The LED D3 is also set high when the pump is on.
 *
 *         The function first sets the air pump pin high, then checks if the
 *         current pressure is greater than or equal to the target pressure. If
 *         it is, it sets the air pump pin low and the LED D3 pin low.
 */
void air_pump(uint32_t pressure)
{
  uint32_t pressure_current = 0;
  uint8_t lp = 1;

  // Turn on the air pump by setting the GPIO pin high
  HAL_GPIO_WritePin(PUMP_GPIO_Port, PUMP_Pin, GPIO_PIN_SET);

  while (lp)
  {
    pressure_current = Get_Pressure();

    // Check if the current pressure is greater than or equal to the target pressure
    if (pressure_current >= pressure)
    {
      // Turn off the air pump by setting the GPIO pin low
      HAL_GPIO_WritePin(PUMP_GPIO_Port, PUMP_Pin, GPIO_PIN_RESET);

      lp = 0;
    }
  }
}

/**
 * @brief  Initializes the HX710B sensor.
 * @retval int: Returns 0 on success, -1 on timeout error.
 */
int Init_HX710B(void)
{
  uint8_t i;

  // Ensure the clock pin (SCK) is low
  HAL_GPIO_WritePin(HX710B_SCK_GPIO_Port, HX710B_SCK_Pin, GPIO_PIN_RESET);

  // Wait for the data pin (DT) to go low, indicating the device is ready
  uint32_t timeout = 100000;
  while (HAL_GPIO_ReadPin(HX710B_DT_GPIO_Port, HX710B_DT_Pin) == GPIO_PIN_SET && timeout--)
  {
    delay_us(1);
  }

  // Check for timeout
  if (timeout == 0)
  {
    return -1; // Timeout occurred, return error
  }

  // Pulse the clock pin 27 times to initialize the sensor
  for (i = 0; i < 27; i++)
  {
    HAL_GPIO_WritePin(HX710B_SCK_GPIO_Port, HX710B_SCK_Pin, GPIO_PIN_SET);
    delay_us(1); // Delay for signal stabilization

    HAL_GPIO_WritePin(HX710B_SCK_GPIO_Port, HX710B_SCK_Pin, GPIO_PIN_RESET);
    delay_us(1); // Delay for signal stabilization
  }

  return 0; // Initialization successful
}

/**
 * @brief  Reads data from the HX710B sensor.
 * @retval int: Returns 32-bit value of the ADC reading, or -1 on timeout error.
 */
uint32_t HX710B_ReadData(void)
{
  uint32_t data = 0;
  uint8_t i;

  /* Pulse the clock pin low to start the conversion */
  HAL_GPIO_WritePin(HX710B_SCK_GPIO_Port, HX710B_SCK_Pin, GPIO_PIN_RESET);

  /* Wait for the data pin to go low, indicating the conversion is complete */
  uint32_t timeout = 100000;
  while (HAL_GPIO_ReadPin(HX710B_DT_GPIO_Port, HX710B_DT_Pin) == GPIO_PIN_SET && timeout--)
  {
    delay_us(1);
  }
  if (timeout == 0)
  {
    return -1; /* timeout occurred */
  }

  /* Read the 24-bit data from the sensor */
  for (i = 0; i < 24; i++)
  {
    /* Pulse the clock pin high to sample the data */
    HAL_GPIO_WritePin(HX710B_SCK_GPIO_Port, HX710B_SCK_Pin, GPIO_PIN_SET);
    delay_us(1);

    /* Read the data pin and shift it into the data variable */
    data = (data << 1) | HAL_GPIO_ReadPin(HX710B_DT_GPIO_Port, HX710B_DT_Pin);

    /* Pulse the clock pin low to prepare for the next sample */
    HAL_GPIO_WritePin(HX710B_SCK_GPIO_Port, HX710B_SCK_Pin, GPIO_PIN_RESET);
    delay_us(1);
  }
  // if (!(data & 0x800000)){
  //   data ^ 0xFF800000;
  // }
  /* Return the 32-bit value, with the MSB set to 0 */
  return data;
}

/**
 * @brief  Retrieves and converts pressure data from the HX710B sensor.
 * @retval uint32_t: The pressure in Pascals.
 * @note   This function reads raw ADC data from the HX710B sensor, converts it to pressure,
 *         and returns the pressure value in Pascals.
 */
uint32_t Get_Pressure(void)
{
  uint32_t pressure = 0;

  // Read raw ADC data from the HX710B sensor
  pressure = HX710B_ReadData();

  // Convert the raw ADC data to pressure in Pascals
  pressure = ConvertToPressure(pressure);

  // Return the converted pressure value
  return pressure;
}

/**
 * @brief  Converts ADC value from HX710B sensor to pressure in mmHg.
 * @param  adc_value: 24-bit unsigned ADC value from HX710B.
 * @retval float: Pressure in mmHg.
 * @note   This function converts the raw ADC data from the HX710B sensor into a signed 32-bit integer,
 *         then calculates the pressure based on calibration values and returns the pressure in mmHg.
 */
float ConvertToPressure(uint32_t adc_value)
{
  // Convert 24-bit unsigned ADC value to signed 32-bit integer
  int32_t signed_adc = (int32_t)(adc_value & 0xFFFFFF);
  if (signed_adc >= 0x800000)
  {
    signed_adc -= 0x1000000; // Convert range from 0 to 16777215 to -8388608 to 8388607
  }

  // Calibration values based on actual measurements
  const float offset = 1359514.0;   // ADC value when pressure = 0 mmHg (calculated)
  const float adc_max = 8388607.0;  // ADC value at maximum pressure of 300 mmHg (assumed)
  const float pressure_max = 300.0; // Maximum pressure range in mmHg

  // Slope for conversion from ADC value to pressure
  static const float slope = pressure_max / (adc_max - offset);

  // Calculate relative pressure
  float pressure = (signed_adc - offset) * slope;

  // Limit negative pressure values to zero
  return (pressure < 0.0) ? 0.0 : pressure;
}

/**
 * @brief  Prints a string to the serial console via UART.
 * @param  str: The string to print.
 * @retval None
 */
void uart_print(const char *str)
{
  /* Send the string to the serial console using the UART HAL driver */
  HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

/**
 * @brief  Interrupt handler for EXTI3 (Button K1).
 * @note   This interrupt handler is called when the button is pressed.
 * @retval None
 */
void EXTI3_IRQHandler(void)
{
  /* Call the HAL GPIO EXTI interrupt handler to handle the interrupt */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

/**
 * @brief  Callback function for handling GPIO EXTI interrupts.
 * @param  GPIO_Pin: Specifies the pin connected to the EXTI line.
 * @retval None
 * @note   This function is triggered when an external interrupt occurs on a GPIO pin.
 *         It debounces the button press by checking the time elapsed since the last interrupt.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  static uint32_t last_time = 0;         // Store the last interrupt time
  uint32_t current_time = HAL_GetTick(); // Get the current system tick

  // Check if the interrupt is from pin 3 and if debounce time has passed
  if (GPIO_Pin == GPIO_PIN_3 && (current_time - last_time > 20))
  {
    button_pressed_k1 = 1;    // Set flag to indicate button K1 was pressed
    last_time = current_time; // Update the last interrupt time
  }
}

/**
 * @brief  Controls the pump by toggling its GPIO pin.
 * @retval None
 */
void control_pump(void)
{
  // Toggle the state of the pump's GPIO pin
  HAL_GPIO_TogglePin(PUMP_GPIO_Port, PUMP_Pin);
}

/**
 * @brief  Exhales air by setting the PWM output to 80% of the maximum value.
 * @retval None
 */
void exhaust(void)
{
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 200); // 20% of 999
}

/**
 * @brief  Stops exhaling by setting the PWM output to 100% of the maximum value.
 *         This function is used to stop exhaling when the user presses button K1.
 * @retval None
 */
void stop_exhaust(void)
{
  /* Set the PWM output to 0% of the maximum value to stop exhaling */
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0); // 0% of 999
}

/**
 * @brief  Fully opens the air valve to exhaust all air.
 * @retval None
 * @details
 *         This function sets the PWM output to 100% of the maximum
 *         value, fully opening the valve to release all air.
 */
void exhaust_all(void)
{
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 999); // Set PWM to maximum (100%)
}

void start_sampling(void)
{
  stop_exhaust();
  air_pump(180);
  delay_us(500);
}

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
 * @brief  End the sampling operation.
 * @note   This function is called after the sampling operation is complete.
 * @retval None
 */
void end_sample(void)
{
  float systolic = 0, diastolic = 0;
  char buffer[50];
  // Exhaust all air from the cuff to stop the sampling operation
  exhaust_all();
  process_data(sample_list, SAMPLE_NUMBER, &systolic, &diastolic);
  sprintf(buffer, "Systolic: %.1f mmHg, Diastolic: %.1f mmHg\r\n", systolic, diastolic);
  HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
}

void blood_pressure_measurement(void)
{
  // Start the sampling process
  start_sampling();

  // Sample the pressure data
  sample();

  // End the sampling process and calculate blood pressure
  end_sample();
}

/**
 * @brief  Calibrates the pressure by controlling the air pump and monitoring pressure levels.
 * @note   This function initializes the calibration process, controls the air pump, monitors pressure,
 *         and provides feedback through UART and LEDs.
 * @retval None
 */
void Pressure_Calibration(void)
{
  uint8_t lp = 1; // Loop control variable
  uint32_t pressure = 0;

  // Stop any ongoing exhaust operation to ensure pump readiness
  stop_exhaust();

  // Turn on LED D3 to indicate the cuff is open
  HAL_GPIO_WritePin(LED_D3_GPIO_Port, LED_D3_Pin, GPIO_PIN_RESET);

  // Start the air pump to build pressure to a target threshold
  air_pump(180);

  // Allow time for the pressure to build up
  HAL_Delay(5000);

  // Loop to check the pressure and provide user feedback
  while (lp)
  {
    HAL_Delay(1000 / 40);
    pressure = Get_Pressure();
    // Check if the pressure is below the threshold value
    if (pressure < 180)
    {
      // If pressure is below threshold, prompt to open cuff and retry calibration
      // uart_print("Open cuff\n");
      HAL_UART_Transmit(&huart1, (uint8_t *)&pressure, sizeof(pressure), HAL_MAX_DELAY);
      HAL_Delay(1000);
    }
    else
    {
      // If pressure is sufficient, indicate cuff is closed
      // uart_print("Close cuff\n");
      HAL_UART_Transmit(&huart1, (uint8_t *)&pressure, sizeof(pressure), HAL_MAX_DELAY);

      // Turn off LED D3 to indicate the cuff is closed
      HAL_GPIO_WritePin(LED_D3_GPIO_Port, LED_D3_Pin, GPIO_PIN_SET);

      // Exhaust all pressure to complete calibration
      exhaust_all();

      // Exit the loop as calibration is complete
      lp = 0;
    }
  }
}

/**
 * @brief Processes the input data to calculate systolic and diastolic values.
 *
 * This function computes the DC component, AC component, and the envelope of the AC component
 * from the input data. It then determines the maximum amplitude position (MAP),
 * systolic, and diastolic indices, and returns the corresponding DC values as systolic
 * and diastolic pressures.
 *
 * @param data Pointer to the input data array.
 * @param size The size of the input data array.
 * @param systolic Pointer to store the calculated systolic value.
 * @param diastolic Pointer to store the calculated diastolic value.
 */
void process_data(float *data, uint16_t size, float *systolic, float *diastolic)
{
  float dc[size], ac[size], envelope[size];

  // Calculate the DC component
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

  // Calculate the envelope of the AC component
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

  // Find the maximum amplitude position (MAP)
  int max_index = 0;
  for (int i = 1; i < size; i++)
  {
    if (envelope[i] > envelope[max_index])
      max_index = i;
  }
  float max_envelope = envelope[max_index];

  // Find the systolic index
  int systolic_index = max_index;
  while (systolic_index > 0 && envelope[systolic_index] >= RS * max_envelope)
  {
    systolic_index--;
  }

  // Find the diastolic index
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

#ifdef USE_FULL_ASSERT
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
