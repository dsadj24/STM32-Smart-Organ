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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NOTE_COUNT      8
#define KEY_DEBOUNCE_MS 10
#define KEY_PORT        GPIOA
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* 音符频率对应的 ARR 值 (PSC=71, 72MHz/(72*freq)-1) */
uint16_t Note_ARR[NOTE_COUNT] = {3821, 3404, 3033, 2863, 2550, 2272, 2024, 1910};

/* 音符名称 */
const char *Note_Names[NOTE_COUNT] = {"Do  ", "Re  ", "Mi  ", "Fa  ", "Sol ", "La  ", "Si  ", "Do' "};

/* 《小星星》音符序列 */
uint8_t Star_Notes[] = {0, 0, 4, 4, 5, 5, 4, 3, 3, 2, 2, 1, 1, 0, 
                        4, 4, 3, 3, 2, 2, 1, 4, 4, 3, 3, 2, 2, 1, 
                        0, 0, 4, 4, 5, 5, 4, 3, 3, 2, 2, 1, 1, 0};
uint16_t Star_Times[] = {500, 500, 500, 500, 500, 500, 1000, 500, 500, 500, 500, 500, 500, 1000,
                         500, 500, 500, 500, 500, 500, 1000, 500, 500, 500, 500, 500, 500, 1000,
                         500, 500, 500, 500, 500, 500, 1000, 500, 500, 500, 500, 500, 500, 1000};
#define STAR_NOTE_COUNT (sizeof(Star_Notes)/sizeof(Star_Notes[0]))

/* 《生日快乐》音符序列 */
uint8_t Bday_Notes[] = {4, 4, 5, 4, 7, 6, 4, 4, 5, 4, 7, 6, 
                        4, 4, 0, 2, 1, 6, 5, 3, 3, 2, 1, 6, 5};
uint16_t Bday_Times[] = {500, 500, 1000, 500, 500, 1000, 500, 500, 1000, 500, 500, 1000,
                         500, 500, 1000, 500, 500, 500, 1000, 500, 500, 1000, 500, 500, 1000};
#define BDAY_NOTE_COUNT (sizeof(Bday_Notes)/sizeof(Bday_Notes[0]))

uint8_t rx_data = 0;
uint8_t last_key = 0xFF;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Play_Note(uint8_t note_index, uint16_t duration);
void Play_Twinkle_Star(void);
void Play_Birthday(void);
void Auto_Play_Songs(void);
uint8_t Read_Key(void);
void OLED_ShowStatusLine(const char *status);
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
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  
  OLED_Init();
  OLED_Clear();
  
  OLED_ShowString(28, 0, (uint8_t*)"SWUST Design");
  OLED_ShowString(32, 2, (uint8_t*)"Smart Organ");
  OLED_ShowString(20, 4, (uint8_t*)"By: Jin ChaoXiang");
  HAL_Delay(2000);
  
  OLED_Clear();
  OLED_ShowString(32, 6, (uint8_t*)"Status: Idle    ");
  
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
  
  HAL_UART_Receive_IT(&huart1, &rx_data, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (HAL_GPIO_ReadPin(KEY_PORT, GPIO_PIN_1) == GPIO_PIN_RESET && 
        HAL_GPIO_ReadPin(KEY_PORT, GPIO_PIN_7) == GPIO_PIN_RESET)
    {
      HAL_Delay(KEY_DEBOUNCE_MS);
      if (HAL_GPIO_ReadPin(KEY_PORT, GPIO_PIN_1) == GPIO_PIN_RESET && 
          HAL_GPIO_ReadPin(KEY_PORT, GPIO_PIN_7) == GPIO_PIN_RESET)
      {
        Auto_Play_Songs();
        while (HAL_GPIO_ReadPin(KEY_PORT, GPIO_PIN_1) == GPIO_PIN_RESET || 
               HAL_GPIO_ReadPin(KEY_PORT, GPIO_PIN_7) == GPIO_PIN_RESET);
        OLED_ShowStatusLine("Status: Idle    ");
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
        last_key = 0xFF;
        continue;
      }
    }
    
    uint8_t key = Read_Key();
    
    if (key != 0xFF)
    {
      uint16_t arr_val = Note_ARR[key];
      __HAL_TIM_SET_AUTORELOAD(&htim2, arr_val);
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, arr_val / 10);
      
      char buf[32];
      sprintf(buf, "Playing: %d %s", key + 1, Note_Names[key]);
      OLED_ShowStatusLine(buf);
      
      last_key = key;
      while (Read_Key() == key);
    }
    else if (last_key != 0xFF)
    {
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
      OLED_ShowStatusLine("Status: Idle    ");
      last_key = 0xFF;
    }
    
    HAL_Delay(5);
  }
  /* USER CODE END WHILE */
  /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

uint8_t Read_Key(void)
{
  uint8_t i;
  for (i = 0; i < NOTE_COUNT; i++)
  {
    if (HAL_GPIO_ReadPin(KEY_PORT, GPIO_PIN_1 << i) == GPIO_PIN_RESET)
    {
      HAL_Delay(KEY_DEBOUNCE_MS);
      if (HAL_GPIO_ReadPin(KEY_PORT, GPIO_PIN_1 << i) == GPIO_PIN_RESET)
      {
        return i;
      }
    }
  }
  return 0xFF;
}

void Play_Note(uint8_t note_index, uint16_t duration)
{
  if (note_index >= NOTE_COUNT) return;
  
  uint16_t arr_val = Note_ARR[note_index];
  __HAL_TIM_SET_AUTORELOAD(&htim2, arr_val);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, arr_val / 10);
  HAL_Delay(duration);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
  HAL_Delay(30);
}

void OLED_ShowStatusLine(const char *status)
{
  OLED_SetPos(0, 6);
  for (uint8_t i = 0; i < 128; i++)
  {
    OLED_WriteData(0x00);
  }
  OLED_SetPos(0, 7);
  for (uint8_t i = 0; i < 128; i++)
  {
    OLED_WriteData(0x00);
  }
  OLED_ShowString(0, 6, (uint8_t*)status);
}

void Play_Twinkle_Star(void)
{
  uint8_t i;
  for (i = 0; i < STAR_NOTE_COUNT; i++)
  {
    Play_Note(Star_Notes[i], Star_Times[i]);
  }
}

void Play_Birthday(void)
{
  uint8_t i;
  for (i = 0; i < BDAY_NOTE_COUNT; i++)
  {
    Play_Note(Bday_Notes[i], Bday_Times[i]);
  }
}

void Auto_Play_Songs(void)
{
  OLED_Clear();
  
  OLED_ShowStatusLine("Music: Twinkle* ");
  Play_Twinkle_Star();
  
  HAL_Delay(500);
  
  OLED_ShowStatusLine("Music: HappyBDay");
  Play_Birthday();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    if (rx_data >= '0' && rx_data <= '7')
    {
      uint8_t note_idx = rx_data - '0';
      Play_Note(note_idx, 300);
    }
    HAL_UART_Receive_IT(&huart1, &rx_data, 1);
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
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */