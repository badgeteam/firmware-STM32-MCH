/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Vusb_Pin GPIO_PIN_0
#define Vusb_GPIO_Port GPIOA
#define Vbat_Pin GPIO_PIN_1
#define Vbat_GPIO_Port GPIOA
#define SAO_IO3_Pin GPIO_PIN_0
#define SAO_IO3_GPIO_Port GPIOB
#define SAO_IO2_Pin GPIO_PIN_1
#define SAO_IO2_GPIO_Port GPIOB
#define SAO_IO1_Pin GPIO_PIN_2
#define SAO_IO1_GPIO_Port GPIOB
#define SAO_IO0_Pin GPIO_PIN_10
#define SAO_IO0_GPIO_Port GPIOB
#define EXT_IO0_Pin GPIO_PIN_12
#define EXT_IO0_GPIO_Port GPIOB
#define EXT_IO1_Pin GPIO_PIN_13
#define EXT_IO1_GPIO_Port GPIOB
#define LED_PWR_Pin GPIO_PIN_14
#define LED_PWR_GPIO_Port GPIOB
#define LCD_RESET_Pin GPIO_PIN_4
#define LCD_RESET_GPIO_Port GPIOB
#define LCD_MODE_Pin GPIO_PIN_5
#define LCD_MODE_GPIO_Port GPIOB
#define LCD_REQUEST_Pin GPIO_PIN_8
#define LCD_REQUEST_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define BUTTON_TICK 5
#define UART_SERIAL huart2
#define UART_FPGA huart1

//#define UART_WEBUSB huart2

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
