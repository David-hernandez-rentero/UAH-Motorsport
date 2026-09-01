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
#include "gpio.h"                                                                                                       // Incluye las definiciones de pines y puertos GPIO declaradas para la aplicación.

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

void MX_GPIO_Init(void)                                                                                                 // Inicializa los GPIO utilizados por la unidad Ready To Drive.
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};                                                                               // Estructura HAL empleada para configurar modo, polarización y velocidad de los GPIO.

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();                                                                                         // Habilita el reloj del puerto GPIOC antes de configurar sus pines.
  __HAL_RCC_GPIOF_CLK_ENABLE();                                                                                         // Habilita el reloj del puerto GPIOF.
  __HAL_RCC_GPIOA_CLK_ENABLE();                                                                                         // Habilita el reloj del puerto GPIOA.
  __HAL_RCC_GPIOB_CLK_ENABLE();                                                                                         // Habilita el reloj del puerto GPIOB.

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED_AVISO2_Pin|OUTPUT_AIR_HIGH_Pin|LED_AVISO1_Pin, GPIO_PIN_RESET);                          // Fija inicialmente a nivel bajo los LEDs y la salida de mando del AIR+.

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, OUTPUT_AIR_LOW_Pin|OUTPUT_DISCHARGE_Pin, GPIO_PIN_RESET);                                    // Fija inicialmente a nivel bajo las salidas de mando del AIR- y descarga.

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OUTPUT_PRECHARGE_GPIO_Port, OUTPUT_PRECHARGE_Pin, GPIO_PIN_RESET);                                  // Fija inicialmente a nivel bajo la salida de mando del relé de precarga.

  /*Configure GPIO pins : LED_AVISO2_Pin OUTPUT_AIR_HIGH_Pin LED_AVISO1_Pin */
  GPIO_InitStruct.Pin = LED_AVISO2_Pin|OUTPUT_AIR_HIGH_Pin|LED_AVISO1_Pin;                                              // Selecciona los pines de GPIOC que se configurarán como salidas digitales.
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;                                                                           // Configura las salidas seleccionadas en modo push-pull.
  GPIO_InitStruct.Pull = GPIO_NOPULL;                                                                                   // Deshabilita resistencias internas pull-up y pull-down en estas salidas.
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                                                                          // Selecciona una velocidad de conmutación baja para las salidas.
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);                                                                               // Aplica la configuración definida a los pines seleccionados del puerto GPIOC.

  /*Configure GPIO pin : INPUT_SC_Pin */
  GPIO_InitStruct.Pin = INPUT_SC_Pin;                                                                                   // Selecciona la entrada correspondiente al circuito de seguridad.
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;                                                                   // Configura la entrada de seguridad para generar interrupción en ambos flancos.
  GPIO_InitStruct.Pull = GPIO_NOPULL;                                                                                   // No aplica polarización interna a la entrada del circuito de seguridad.
  HAL_GPIO_Init(INPUT_SC_GPIO_Port, &GPIO_InitStruct);                                                                  // Aplica la configuración a la entrada de seguridad.

  /*Configure GPIO pins : OUTPUT_AIR_LOW_Pin OUTPUT_DISCHARGE_Pin */
  GPIO_InitStruct.Pin = OUTPUT_AIR_LOW_Pin|OUTPUT_DISCHARGE_Pin;                                                        // Selecciona las salidas de mando del AIR- y del circuito de descarga.
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;                                                                           // Configura ambas señales como salidas digitales push-pull.
  GPIO_InitStruct.Pull = GPIO_NOPULL;                                                                                   // Deshabilita resistencias internas en las salidas AIR- y descarga.
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                                                                          // Selecciona velocidad baja de conmutación.
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                                                                               // Inicializa las salidas AIR- y descarga en el puerto GPIOA.

  /*Configure GPIO pin : OUTPUT_PRECHARGE_Pin */
  GPIO_InitStruct.Pin = OUTPUT_PRECHARGE_Pin;                                                                           // Selecciona el pin utilizado para gobernar el relé de precarga.
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;                                                                           // Configura el mando de precarga como salida push-pull.
  GPIO_InitStruct.Pull = GPIO_NOPULL;                                                                                   // Mantiene la salida de precarga sin polarización interna.
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                                                                          // Configura la salida de precarga con velocidad de conmutación baja.
  HAL_GPIO_Init(OUTPUT_PRECHARGE_GPIO_Port, &GPIO_InitStruct);                                                          // Aplica la configuración al pin de mando de precarga.

  /*Configure GPIO pin : INPUT_AIR_LOW_Pin */
  GPIO_InitStruct.Pin = INPUT_AIR_LOW_Pin;                                                                              // Selecciona la entrada de realimentación del AIR-.
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;                                                                               // Configura el feedback del AIR- como entrada digital.
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;                                                                                 // Activa una resistencia pull-down interna en la entrada.
  HAL_GPIO_Init(INPUT_AIR_LOW_GPIO_Port, &GPIO_InitStruct);                                                             // Aplica la configuración al pin de feedback del AIR-.

  /*Configure GPIO pins : INPUT_PRECHARGE_Pin INPUT_AIR_HIGH_Pin */
  GPIO_InitStruct.Pin = INPUT_PRECHARGE_Pin|INPUT_AIR_HIGH_Pin;                                                         // Selecciona conjuntamente las entradas de feedback de precarga y AIR+.
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;                                                                               // Configura ambos feedbacks como entradas digitales.
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;                                                                                 // Activa resistencias pull-down internas para ambas entradas.
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                                                                               // Aplica la configuración de las entradas de feedback en GPIOA.

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);                                                                               // Establece la máxima prioridad para la interrupción EXTI asociada a INPUT_SC.
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);                                                                                       // Habilita en el NVIC la interrupción externa EXTI3.

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
