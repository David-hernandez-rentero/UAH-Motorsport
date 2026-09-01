/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dac.c
  * @brief   Configuración del convertidor digital-analógico (DAC) del AMS.
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
#include "dac.h"                                                                            // Declaración del periférico DAC y funciones HAL asociadas

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

DAC_HandleTypeDef hdac1;                                                                    // Estructura de control utilizada por la HAL para gestionar DAC1

/* DAC1 init function */
void MX_DAC1_Init(void)
{
  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};                                                     // Estructura que almacena la configuración del canal DAC

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */

  /* Inicialización general del periférico DAC1 */
  hdac1.Instance = DAC1;                                                                    // Selecciona la instancia DAC1 del microcontrolador
  if (HAL_DAC_Init(&hdac1) != HAL_OK)                                                       // Inicializa DAC1 mediante la biblioteca HAL
  {
    Error_Handler();                                                                        // Ejecuta el gestor de error si la inicialización falla
  }

  /* Configuración del canal DAC1_OUT1 */
  sConfig.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_AUTOMATIC;                   // Ajusta automáticamente el modo de interfaz para alta frecuencia
  sConfig.DAC_DMADoubleDataMode = DISABLE;                                                  // Deshabilita el modo de doble dato mediante DMA
  sConfig.DAC_SignedFormat = DISABLE;                                                       // Deshabilita el formato de datos con signo
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;                                    // Deshabilita el modo Sample and Hold
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;                                                   // No utiliza una señal externa para disparar la conversión
  sConfig.DAC_Trigger2 = DAC_TRIGGER_NONE;                                                  // Mantiene deshabilitado el segundo disparador del canal
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;                                       // Habilita el buffer de salida del DAC
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_BOTH;                               // Conecta la salida al pin externo y a los periféricos internos
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;                                          // Utiliza los valores de calibración establecidos en fábrica

  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)                      // Aplica la configuración anterior al canal 1 de DAC1
  {
    Error_Handler();                                                                        // Ejecuta el gestor de error si la configuración falla
  }

  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */
}

void HAL_DAC_MspInit(DAC_HandleTypeDef* dacHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};                                                   // Estructura empleada para configurar el GPIO asociado al DAC

  if (dacHandle->Instance == DAC1)                                                          // Comprueba que la instancia inicializada corresponde a DAC1
  {
    /* USER CODE BEGIN DAC1_MspInit 0 */

    /* USER CODE END DAC1_MspInit 0 */

    __HAL_RCC_DAC1_CLK_ENABLE();                                                            // Habilita el reloj del periférico DAC1
    __HAL_RCC_GPIOA_CLK_ENABLE();                                                           // Habilita el reloj del puerto GPIOA

    /* DAC1 GPIO Configuration
       PA4     ------> DAC1_OUT1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4;                                                       // Selecciona PA4, asociado a DAC1_OUT1
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;                                                // Configura PA4 como señal analógica
    GPIO_InitStruct.Pull = GPIO_NOPULL;                                                     // Deshabilita resistencias internas de pull-up y pull-down
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                                                 // Aplica la configuración al puerto GPIOA

    /* USER CODE BEGIN DAC1_MspInit 1 */

    /* USER CODE END DAC1_MspInit 1 */
  }
}

void HAL_DAC_MspDeInit(DAC_HandleTypeDef* dacHandle)
{
  if (dacHandle->Instance == DAC1)                                                          // Comprueba que se está desinicializando DAC1
  {
    /* USER CODE BEGIN DAC1_MspDeInit 0 */

    /* USER CODE END DAC1_MspDeInit 0 */

    __HAL_RCC_DAC1_CLK_DISABLE();                                                           // Deshabilita el reloj de DAC1
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);                                                     // Libera la configuración analógica del pin PA4

    /* USER CODE BEGIN DAC1_MspDeInit 1 */

    /* USER CODE END DAC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
