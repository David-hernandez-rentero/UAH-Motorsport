/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_AVISO2_Pin GPIO_PIN_13
#define LED_AVISO2_GPIO_Port GPIOC
#define OUTPUT_AIR_HIGH_Pin GPIO_PIN_1
#define OUTPUT_AIR_HIGH_GPIO_Port GPIOC
#define LED_AVISO1_Pin GPIO_PIN_2
#define LED_AVISO1_GPIO_Port GPIOC
#define INPUT_SC_Pin GPIO_PIN_3
#define INPUT_SC_GPIO_Port GPIOC
#define INPUT_SC_EXTI_IRQn EXTI3_IRQn
#define OUTPUT_AIR_LOW_Pin GPIO_PIN_1
#define OUTPUT_AIR_LOW_GPIO_Port GPIOA
#define OUTPUT_DISCHARGE_Pin GPIO_PIN_4
#define OUTPUT_DISCHARGE_GPIO_Port GPIOA
#define OUTPUT_PRECHARGE_Pin GPIO_PIN_0
#define OUTPUT_PRECHARGE_GPIO_Port GPIOB
#define INPUT_AIR_LOW_Pin GPIO_PIN_7
#define INPUT_AIR_LOW_GPIO_Port GPIOC
#define INPUT_PRECHARGE_Pin GPIO_PIN_8
#define INPUT_PRECHARGE_GPIO_Port GPIOA
#define INPUT_AIR_HIGH_Pin GPIO_PIN_9
#define INPUT_AIR_HIGH_GPIO_Port GPIOA
#define T_SWDIO_Pin GPIO_PIN_13
#define T_SWDIO_GPIO_Port GPIOA
#define T_SWCLK_Pin GPIO_PIN_14
#define T_SWCLK_GPIO_Port GPIOA
#define T_SWO_Pin GPIO_PIN_3
#define T_SWO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
