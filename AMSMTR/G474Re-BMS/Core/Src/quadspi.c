/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    quadspi.c
  * @brief   This file provides code for the configuration
  *          of the QUADSPI instances.
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
#include "quadspi.h"                                                          // Incluye las definiciones y funciones HAL asociadas al periférico QUADSPI.

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

QSPI_HandleTypeDef hqspi1;                                                    // Estructura de control utilizada por la HAL para gestionar el periférico QUADSPI.

/* QUADSPI1 init function */
void MX_QUADSPI1_Init(void)
{

  /* USER CODE BEGIN QUADSPI1_Init 0 */

  /* USER CODE END QUADSPI1_Init 0 */

  /* USER CODE BEGIN QUADSPI1_Init 1 */

  /* USER CODE END QUADSPI1_Init 1 */
  hqspi1.Instance = QUADSPI;                                                  // Selecciona la instancia QUADSPI del microcontrolador.
  hqspi1.Init.ClockPrescaler = 255;                                           // Establece el prescaler del reloj del periférico QUADSPI.
  hqspi1.Init.FifoThreshold = 1;                                              // Define el umbral de la FIFO en un byte.
  hqspi1.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;                     // Deshabilita el desplazamiento del instante de muestreo.
  hqspi1.Init.FlashSize = 1;                                                  // Configura el tamaño lógico de la memoria externa gestionada por QUADSPI.
  hqspi1.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;                 // Mantiene la señal Chip Select inactiva durante un ciclo entre accesos.
  hqspi1.Init.ClockMode = QSPI_CLOCK_MODE_0;                                  // Configura el reloj de comunicación en modo 0.
  hqspi1.Init.FlashID = QSPI_FLASH_ID_1;                                      // Selecciona el banco de memoria Flash 1.
  hqspi1.Init.DualFlash = QSPI_DUALFLASH_DISABLE;                             // Deshabilita el funcionamiento con dos memorias Flash en paralelo.
  if (HAL_QSPI_Init(&hqspi1) != HAL_OK)                                       // Inicializa QUADSPI con los parámetros configurados y comprueba posibles errores.
  {
    Error_Handler();                                                          // Ejecuta el gestor de error si falla la operación anterior.
  }
  /* USER CODE BEGIN QUADSPI1_Init 2 */

  /* USER CODE END QUADSPI1_Init 2 */

}

void HAL_QSPI_MspInit(QSPI_HandleTypeDef* qspiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};                                     // Estructura utilizada para configurar los pines GPIO del periférico.
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};                               // Estructura utilizada para seleccionar la fuente de reloj de QUADSPI.
  if(qspiHandle->Instance==QUADSPI)                                           // Comprueba que la instancia gestionada corresponde al periférico QUADSPI.
  {
  /* USER CODE BEGIN QUADSPI_MspInit 0 */

  /* USER CODE END QUADSPI_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_QSPI;                  // Selecciona el reloj correspondiente al periférico QUADSPI.
    PeriphClkInit.QspiClockSelection = RCC_QSPICLKSOURCE_SYSCLK;              // Utiliza SYSCLK como fuente de reloj para QUADSPI.

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)                  // Aplica la configuración del reloj y comprueba posibles errores.
    {
      Error_Handler();                                                        // Ejecuta el gestor de error si falla la operación anterior.
    }

    /* QUADSPI clock enable */
    __HAL_RCC_QSPI_CLK_ENABLE();                                              // Habilita el reloj del periférico QUADSPI.

    __HAL_RCC_GPIOA_CLK_ENABLE();                                             // Habilita el reloj del puerto GPIOA.
    __HAL_RCC_GPIOB_CLK_ENABLE();                                             // Habilita el reloj del puerto GPIOB.
    /**QUADSPI1 GPIO Configuration
    PA6     ------> QUADSPI1_BK1_IO3
    PA7     ------> QUADSPI1_BK1_IO2
    PB0     ------> QUADSPI1_BK1_IO1
    PB1     ------> QUADSPI1_BK1_IO0
    PB10     ------> QUADSPI1_CLK
    PB11     ------> QUADSPI1_BK1_NCS
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;                              // Selecciona PA6 y PA7 para las líneas IO3 e IO2 de QUADSPI.
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;                                   // Configura los pines en modo de función alternativa push-pull.
    GPIO_InitStruct.Pull = GPIO_NOPULL;                                       // Deshabilita las resistencias internas de pull-up y pull-down.
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                              // Configura una velocidad de conmutación baja para los GPIO.
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;                            // Asigna la función alternativa AF10 correspondiente a QUADSPI.
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                                   // Aplica la configuración QUADSPI a los pines seleccionados del puerto GPIOA.

    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11;      // Selecciona PB0, PB1, PB10 y PB11 para IO1, IO0, CLK y NCS.
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;                                   // Configura los pines en modo de función alternativa push-pull.
    GPIO_InitStruct.Pull = GPIO_NOPULL;                                       // Deshabilita las resistencias internas de pull-up y pull-down.
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                              // Configura una velocidad de conmutación baja para los GPIO.
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;                            // Asigna la función alternativa AF10 correspondiente a QUADSPI.
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);                                   // Aplica la configuración QUADSPI a los pines seleccionados del puerto GPIOB.

  /* USER CODE BEGIN QUADSPI_MspInit 1 */

  /* USER CODE END QUADSPI_MspInit 1 */
  }
}

void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* qspiHandle)
{

  if(qspiHandle->Instance==QUADSPI)                                           // Comprueba que la instancia gestionada corresponde al periférico QUADSPI.
  {
  /* USER CODE BEGIN QUADSPI_MspDeInit 0 */

  /* USER CODE END QUADSPI_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_QSPI_CLK_DISABLE();                                             // Deshabilita el reloj del periférico QUADSPI.

    /**QUADSPI1 GPIO Configuration
    PA6     ------> QUADSPI1_BK1_IO3
    PA7     ------> QUADSPI1_BK1_IO2
    PB0     ------> QUADSPI1_BK1_IO1
    PB1     ------> QUADSPI1_BK1_IO0
    PB10     ------> QUADSPI1_CLK
    PB11     ------> QUADSPI1_BK1_NCS
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_6|GPIO_PIN_7);                            // Libera la configuración QUADSPI de los pines PA6 y PA7.

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11);    // Libera la configuración QUADSPI de los pines PB0, PB1, PB10 y PB11.

  /* USER CODE BEGIN QUADSPI_MspDeInit 1 */

  /* USER CODE END QUADSPI_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

