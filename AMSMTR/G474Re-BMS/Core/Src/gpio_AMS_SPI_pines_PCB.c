/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    gpio.c
 * @brief   This file provides code for the configuration
 *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
 * @brief Configures all GPIO pins used by the AMS.
 */
void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();                                                       // Enables GPIOC for SPI2/SPI3 Chip Select signals
  __HAL_RCC_GPIOF_CLK_ENABLE();                                                       // Enables GPIOF for board peripheral functions
  __HAL_RCC_GPIOA_CLK_ENABLE();                                                       // Enables GPIOA for SPI1 CS, ADBMS INTR and WAKE
  __HAL_RCC_GPIOB_CLK_ENABLE();                                                       // Enables GPIOB for the remaining board functions

  /* Initial output levels */
  HAL_GPIO_WritePin(SPI1_CS_OUT_GPIO_Port, SPI1_CS_OUT_Pin, GPIO_PIN_SET);            // Keeps SPI1 Chip Select inactive at startup (active LOW)
  HAL_GPIO_WritePin(SPI2_CS_OUT_GPIO_Port, SPI2_CS_OUT_Pin, GPIO_PIN_SET);            // Keeps SPI2 Chip Select inactive at startup (active LOW)
  HAL_GPIO_WritePin(SPI3_CS_OUT_GPIO_Port, SPI3_CS_OUT_Pin, GPIO_PIN_SET);            // Keeps SPI3 Chip Select inactive at startup (active LOW)
  HAL_GPIO_WritePin(ADBMS_WAKE_OUT_GPIO_Port, ADBMS_WAKE_OUT_Pin, GPIO_PIN_RESET);    // Keeps the dedicated ADBMS WAKE output LOW at startup

  /* Configure PA8 (SPI1_CS_OUT) and PA10 (ADBMS_WAKE_OUT) as digital outputs */
  GPIO_InitStruct.Pin = SPI1_CS_OUT_Pin | ADBMS_WAKE_OUT_Pin;                         // Selects PA8 and PA10
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;                                         // Configures both pins as push-pull outputs
  GPIO_InitStruct.Pull = GPIO_NOPULL;                                                 // Disables internal pull-up and pull-down resistors
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                                        // Low switching speed is sufficient for control signals
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                                             // Applies the configuration to GPIOA

  /* Configure PC13 (SPI2_CS_OUT) and PC9 (SPI3_CS_OUT) as digital outputs */
  GPIO_InitStruct.Pin = SPI2_CS_OUT_Pin | SPI3_CS_OUT_Pin;                            // Selects PC13 and PC9
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;                                         // Configures both pins as push-pull outputs
  GPIO_InitStruct.Pull = GPIO_NOPULL;                                                 // Disables internal pull-up and pull-down resistors
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                                        // Low switching speed is sufficient for Chip Select signals
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);                                             // Applies the configuration to GPIOC

  /* Configure PA9 (ADBMS_INTR1_IN) as a digital input */
  GPIO_InitStruct.Pin = ADBMS_INTR1_IN_Pin;                                           // Selects PA9
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;                                             // Configures PA9 as a normal GPIO input
  GPIO_InitStruct.Pull = GPIO_NOPULL;                                                 // Leaves the interrupt input without an internal pull resistor
  HAL_GPIO_Init(ADBMS_INTR1_IN_GPIO_Port, &GPIO_InitStruct);                          // Applies the configuration to the ADBMS interrupt pin
}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 *//* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    gpio.c
 * @brief   This file provides code for the configuration
 *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
 * @brief Configures all GPIO pins used by the AMS.
 */
void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();                                                       // Enables GPIOC for SPI2/SPI3 Chip Select signals
  __HAL_RCC_GPIOF_CLK_ENABLE();                                                       // Enables GPIOF for board peripheral functions
  __HAL_RCC_GPIOA_CLK_ENABLE();                                                       // Enables GPIOA for SPI1 CS, ADBMS INTR and WAKE
  __HAL_RCC_GPIOB_CLK_ENABLE();                                                       // Enables GPIOB for the remaining board functions

  /* Initial output levels */
  HAL_GPIO_WritePin(SPI1_CS_OUT_GPIO_Port, SPI1_CS_OUT_Pin, GPIO_PIN_SET);            // Keeps SPI1 Chip Select inactive at startup (active LOW)
  HAL_GPIO_WritePin(SPI2_CS_OUT_GPIO_Port, SPI2_CS_OUT_Pin, GPIO_PIN_SET);            // Keeps SPI2 Chip Select inactive at startup (active LOW)
  HAL_GPIO_WritePin(SPI3_CS_OUT_GPIO_Port, SPI3_CS_OUT_Pin, GPIO_PIN_SET);            // Keeps SPI3 Chip Select inactive at startup (active LOW)
  HAL_GPIO_WritePin(ADBMS_WAKE_OUT_GPIO_Port, ADBMS_WAKE_OUT_Pin, GPIO_PIN_RESET);    // Keeps the dedicated ADBMS WAKE output LOW at startup

  /* Configure PA8 (SPI1_CS_OUT) and PA10 (ADBMS_WAKE_OUT) as digital outputs */
  GPIO_InitStruct.Pin = SPI1_CS_OUT_Pin | ADBMS_WAKE_OUT_Pin;                         // Selects PA8 and PA10
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;                                         // Configures both pins as push-pull outputs
  GPIO_InitStruct.Pull = GPIO_NOPULL;                                                 // Disables internal pull-up and pull-down resistors
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                                        // Low switching speed is sufficient for control signals
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                                             // Applies the configuration to GPIOA

  /* Configure PC13 (SPI2_CS_OUT) and PC9 (SPI3_CS_OUT) as digital outputs */
  GPIO_InitStruct.Pin = SPI2_CS_OUT_Pin | SPI3_CS_OUT_Pin;                            // Selects PC13 and PC9
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;                                         // Configures both pins as push-pull outputs
  GPIO_InitStruct.Pull = GPIO_NOPULL;                                                 // Disables internal pull-up and pull-down resistors
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                                        // Low switching speed is sufficient for Chip Select signals
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);                                             // Applies the configuration to GPIOC

  /* Configure PA9 (ADBMS_INTR1_IN) as a digital input */
  GPIO_InitStruct.Pin = ADBMS_INTR1_IN_Pin;                                           // Selects PA9
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;                                             // Configures PA9 as a normal GPIO input
  GPIO_InitStruct.Pull = GPIO_NOPULL;                                                 // Leaves the interrupt input without an internal pull resistor
  HAL_GPIO_Init(ADBMS_INTR1_IN_GPIO_Port, &GPIO_InitStruct);                          // Applies the configuration to the ADBMS interrupt pin
}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */