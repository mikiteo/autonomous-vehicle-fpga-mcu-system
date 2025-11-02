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
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"
#include "img_objects.h"
#include <stdio.h>
#include <stdlib.h>
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
void DrawCar(uint16_t y);
uint16_t MapY(uint16_t d);
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
   uint8_t spi_data[6];  // dynamically allocate space

  /* USER CODE END 1 */

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

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
  MX_USART3_UART_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */

		ILI9341_Init();//initial driver setup to drive ili9341
		ILI9341_Draw_Image((const char*)road, SCREEN_VERTICAL_1);


//		ILI9341_Draw_Filled_Circle(120, 35, 10, MAROON);
//		ILI9341_Draw_Filled_Rectangle_Coord(100, 0, 140, 40, GREEN);
//	  	ILI9341_Fill_Screen(WHITE);
//		ILI9341_Set_Rotation(SCREEN_HORIZONTAL_2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//	  	ILI9341_Fill_Screen(WHITE);

	  	volatile HAL_StatusTypeDef spi_status;
	  	spi_status = HAL_SPI_Receive(FPGA_SPI_INST, spi_data, 6, 1000); // Receive 3x16bit data from SPI

	  	uint16_t* data16 = (uint16_t*) spi_data;
	  	int16_t d_prev, v_prev, a_prev;
	  	uint16_t d = ((data16[2] & 0xFF) << 8) | (data16[2] >> 8);
	  	int16_t v = ((data16[0] & 0xFF) << 8) | (data16[0] >> 8);
	  	int16_t a = ((data16[1] & 0xFF) << 8) | (data16[1] >> 8);

	  	if (d > 2400){
	  			d = 2400;
	  		}


	  	char Temp_Buffer_text[40];

	  	if (abs((int16_t)d_prev - (int16_t)d) > 5){
	  		DrawCar(MapY(d));
	  		ILI9341_Set_Rotation(SCREEN_VERTICAL_2);
	  		if (v == 0){
	  			sprintf(Temp_Buffer_text, "v: %d mm/s", v_prev);
	  		} else {
	  			sprintf(Temp_Buffer_text, "v: %d mm/s", v);
	  			v_prev = v;
	  		}
			ILI9341_Draw_Text(Temp_Buffer_text, 10, 30, BLACK, 1, WHITE);

	  		if (a == 0){
	  			sprintf(Temp_Buffer_text, "a: %d mm/s^2", a_prev);
	  		} else {
	  			sprintf(Temp_Buffer_text, "a: %d mm/s^2", a);
	  			a_prev = a;
	  		}
	  			ILI9341_Draw_Text(Temp_Buffer_text, 10, 50, BLACK, 1, WHITE);
	  		}

	  	d_prev = d;
	  	ILI9341_Set_Rotation(SCREEN_VERTICAL_2);
	    sprintf(Temp_Buffer_text, "d: %d mm", d);
	    ILI9341_Draw_Text(Temp_Buffer_text, 10, 10, BLACK, 1, WHITE);


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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 104;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void DrawCar(uint16_t y){
	ILI9341_Draw_Image((const char*)road, SCREEN_VERTICAL_1);
	ILI9341_Draw_Filled_Rectangle_Coord(100, y, 140, y+80, RED);
	ILI9341_Draw_Filled_Rectangle_Coord(105, y+55, 135, y+75, BLACK);
	ILI9341_Draw_Filled_Rectangle_Coord(105, y+15, 135, y+35, BLACK);
}

uint16_t MapY(uint16_t d){
	uint16_t y;
	uint16_t d_max = 2400;
	uint16_t d_min = 10;

	y = (float)(((float)d-d_min)/(d_max-d_min))*240.0;

	return (uint16_t)y + 60;
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
