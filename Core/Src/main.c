/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch){
	HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
	return 0;
}

void ADXL362_WakeUp(){
	uint8_t writeInfo[3] = {0x0A, 0x2D, 0x02};
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, writeInfo, 3, 10);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
}

// status: 0: x axis; 1: y axis; 2: z axis
uint16_t sensor_read(int status){
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
	uint8_t spiCommand = 0x0B;
	HAL_SPI_Transmit(&hspi2, &spiCommand, 1, 10);

	uint8_t axis_register;
	switch(status){
		case 0:
			axis_register = 0x0E;
			break;
		case 1:
			axis_register = 0x10;
			break;
		case 2:
			axis_register = 0x12;
			break;
		default:
			axis_register = 0x00;
			break;
	}
	HAL_SPI_Transmit(&hspi2, &axis_register, 1, 10);
	uint8_t axis_raw[2];
	HAL_SPI_Receive(&hspi2, axis_raw, 2, 10);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

	uint16_t returnValue = axis_raw[0] | (axis_raw[1] << 8);
	printf("return value is: %04X\r\n", returnValue);
	return returnValue;
}

uint8_t read_reg(uint8_t addr){
	uint8_t cmd = 0x0B;
	uint8_t returnValue = 0x5A;   // 刻意給一個特別的初始值
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, &cmd, 1, 10);
	HAL_SPI_Transmit(&hspi2, &addr, 1, 10);
	HAL_SPI_Receive(&hspi2, &returnValue, 1, 10);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
	return returnValue;
}


uint8_t get_sensor_IDRegister(){
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
	uint8_t spiCommand = 0x0B;
	uint8_t register_addr = 0x00;
	uint8_t returnValue;
	HAL_StatusTypeDef s1, s2, s3;

	s1 = HAL_SPI_Transmit(&hspi2, (const uint8_t*) &spiCommand, 1, 10);
	s2 = HAL_SPI_Transmit(&hspi2, (const uint8_t*) &register_addr, 1, 10);
	s3 = HAL_SPI_Receive(&hspi2, &returnValue, 1, 10);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
	return returnValue;
}



float convert_to_SI(int16_t register_data){
	printf("current value is: %d\r\n", register_data);
	return register_data * 0.001f * 9.8;
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
  MX_USART2_UART_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
  printf("Hello, World\r\n");
  HAL_Delay(100);
  printf("ID = 0x%02X\r\n", get_sensor_IDRegister());
  // printf("get_sensor_IDRegister() is: %d\r\n", get_sensor_IDRegister());
  if(get_sensor_IDRegister() == 0xAD){
	  printf("ADXL is connected\r\n");
  }
  else{
	  printf("ADXL is not connected\r\n");
  }
  HAL_Delay(10);
  for (int i = 0; i < 3; i++) {
  	printf("0x00 = 0x%02X  0x01 = 0x%02X  0x02 = 0x%02X\r\n",
  	       read_reg(0x00), read_reg(0x01), read_reg(0x02));
  	HAL_Delay(100);
  }

  /*
  uint8_t tx = 0xA5, rx = 0;
  HAL_SPI_TransmitReceive(&hspi2, &tx, &rx, 1, 10);
  printf("loopback: got 0x%02X\r\n", rx);

  HAL_Delay(10);
  printf("0x00 = 0x%02X (expect 0xAD)\r\n", read_reg(0x00));
  printf("0x01 = 0x%02X (expect 0x1D)\r\n", read_reg(0x01));
  printf("0x02 = 0x%02X (expect 0xF2)\r\n", read_reg(0x02));
  read_reg(0x00);   // 先讓 SPI 啟動一次
  while (1) {       // 暫時的測試迴圈，測完刪掉
  	printf("A0 = %d\r\n", HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0));
  	HAL_Delay(500);
  }
	*/

  /* USER CODE END 2 */

  /* USER CODE BEGIN BSP */

  /* USER CODE END BSP */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  ADXL362_WakeUp();
  while (1)
  {

	  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
	  HAL_Delay(500);
	  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
	  HAL_Delay(500);

	  uint16_t x_acceleration_raw = sensor_read(0);
	  uint16_t y_acceleration_raw = sensor_read(1);
	  uint16_t z_acceleration_raw = sensor_read(2);
	  float x_acceleration_si = convert_to_SI(x_acceleration_raw);
	  float y_acceleration_si = convert_to_SI(y_acceleration_raw);
	  float z_acceleration_si = convert_to_SI(z_acceleration_raw);
	  printf("x_acceleration: %f, y_acceleration: %f, z_acceleration: %f\r\n", x_acceleration_si, y_acceleration_si, z_acceleration_si);

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

/* USER CODE BEGIN 4 */

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
