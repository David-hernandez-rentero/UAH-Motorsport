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
#include "gpio.h"                                                     // GPIO pin declarations and HAL definitions

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)                                               // Enables GPIO ports and configures all application input and output pins
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};                             // Initializes the GPIO configuration structure to zero

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();                                       // Enables the GPIOF peripheral clock
  __HAL_RCC_GPIOC_CLK_ENABLE();                                       // Enables GPIOC for INPUT_SC and ARRANQUE
  __HAL_RCC_GPIOA_CLK_ENABLE();                                       // Enables the GPIOA peripheral clock
  __HAL_RCC_GPIOB_CLK_ENABLE();                                       // Enables GPIOB for the buzzer output

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);    // Forces the buzzer output low before configuring the pin as an output

  /*Configure GPIO pins : INPUT_SC_Pin ARRANQUE_Pin */
  GPIO_InitStruct.Pin = INPUT_SC_Pin|ARRANQUE_Pin;                    // Selects the shutdown-circuit input and R2D start-button input pins
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;                             // Configures the selected pins as digital inputs
  GPIO_InitStruct.Pull = GPIO_NOPULL;                                 // Disables internal pull-up and pull-down resistors
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);                             // Applies the input configuration to the selected GPIOC pins

  /*Configure GPIO pin : BUZZER_Pin */
  GPIO_InitStruct.Pin = BUZZER_Pin;                                   // Selects the buzzer output pin
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;                         // Configures the buzzer pin as a push-pull digital output
  GPIO_InitStruct.Pull = GPIO_NOPULL;                                 // Disables internal pull-up and pull-down resistors
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                        // Uses a low output slew rate because the buzzer signal is slow
  HAL_GPIO_Init(BUZZER_GPIO_Port, &GPIO_InitStruct);                  // Applies the output configuration to the buzzer pin

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
