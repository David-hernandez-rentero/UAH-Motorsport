c:\AMS\Core\Src\crc.c c:\AMS\Core\Src\dac.c/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   Configuración del ADC empleado por el AMS.
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
#include "adc.h"                                                               // Incluye las definiciones y prototipos asociados al periférico ADC.

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;                                                       // Estructura HAL que contiene la configuración y el estado del ADC1.

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};                                        // Estructura utilizada para configurar el modo de funcionamiento entre ADC.
  ADC_ChannelConfTypeDef sConfig = {0};                                        // Estructura utilizada para configurar el canal regular del ADC1.

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;                                                       // Selecciona el periférico ADC1.
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;                        // Divide entre 4 el reloj síncrono utilizado por el ADC.
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;                                  // Configura una resolución de 12 bits: valores digitales entre 0 y 4095.
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;                                  // Alinea el resultado de la conversión a la derecha.
  hadc1.Init.GainCompensation = 0;                                             // No aplica compensación digital de ganancia.
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;                                  // Deshabilita el escaneo, ya que actualmente solo se convierte un canal regular.
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;                               // Genera el fin de conversión al terminar cada conversión individual.
  hadc1.Init.LowPowerAutoWait = DISABLE;                                       // Deshabilita la espera automática de bajo consumo entre conversiones.
  hadc1.Init.ContinuousConvMode = DISABLE;                                     // Deshabilita la conversión continua; cada adquisición debe iniciarse por software.
  hadc1.Init.NbrOfConversion = 1;                                              // Define una única conversión dentro de la secuencia regular.
  hadc1.Init.DiscontinuousConvMode = DISABLE;                                  // Deshabilita el modo discontinuo de conversión.
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;                            // La conversión se inicia mediante una orden software.
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;             // No se emplea ningún flanco externo como disparo de conversión.
  hadc1.Init.DMAContinuousRequests = DISABLE;                                  // Deshabilita las peticiones DMA continuas para la adquisición del ADC.
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;                                 // Conserva el dato anterior si se produce un overrun antes de leer el resultado.
  hadc1.Init.OversamplingMode = DISABLE;                                       // Deshabilita el sobremuestreo interno del ADC.
  if (HAL_ADC_Init(&hadc1) != HAL_OK)                                          // Aplica la configuración general del ADC1 mediante la biblioteca HAL.
  {
    Error_Handler();                                                           // Ejecuta la rutina de error si la inicialización no se completa correctamente.
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;                                       // Configura ADC1 para funcionar de manera independiente respecto a otros ADC.
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)          // Aplica la configuración del modo independiente.
  {
    Error_Handler();                                                           // Ejecuta la rutina de error si falla la configuración multimodo.
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;                                             // Selecciona ADC1_IN1, conectado al pin PA0, como canal regular.
  sConfig.Rank = ADC_REGULAR_RANK_1;                                           // Sitúa ADC1_IN1 en la primera posición de la secuencia de conversión.
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;                             // Establece un tiempo de muestreo de 2,5 ciclos del reloj del ADC.
  sConfig.SingleDiff = ADC_SINGLE_ENDED;                                       // Configura la entrada como single-ended, medida respecto a masa.
  sConfig.OffsetNumber = ADC_OFFSET_NONE;                                      // Deshabilita la aplicación de offset digital sobre el resultado.
  sConfig.Offset = 0;                                                          // Mantiene el valor de offset en cero.
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)                       // Aplica la configuración del canal regular ADC1_IN1.
  {
    Error_Handler();                                                           // Ejecuta la rutina de error si la configuración del canal falla.
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};                                      // Estructura utilizada para configurar los GPIO asociados al ADC.
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};                                // Estructura utilizada para seleccionar la fuente de reloj del ADC12.
  if(adcHandle->Instance==ADC1)                                                // Ejecuta esta configuración únicamente cuando se inicializa ADC1.
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;                  // Selecciona el dominio de reloj correspondiente a ADC1 y ADC2.
    PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;             // Utiliza SYSCLK como fuente de reloj para el bloque ADC12.
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)                   // Aplica la configuración de reloj del periférico ADC12.
    {
      Error_Handler();                                                         // Ejecuta la rutina de error si la configuración del reloj falla.
    }

    /* ADC1 clock enable */
    __HAL_RCC_ADC12_CLK_ENABLE();                                              // Habilita el reloj del bloque ADC12.

    __HAL_RCC_GPIOA_CLK_ENABLE();                                              // Habilita el reloj del puerto GPIOA.
    /**ADC1 GPIO Configuration
    PA0     ------> ADC1_IN1
    PA1     ------> ADC1_IN2
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;                               // Selecciona PA0 y PA1 como pines asociados a ADC1_IN1 y ADC1_IN2.
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;                                   // Configura ambos pines en modo analógico.
    GPIO_InitStruct.Pull = GPIO_NOPULL;                                        // Deshabilita resistencias internas pull-up y pull-down para no alterar las señales.
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                                    // Aplica la configuración analógica sobre PA0 y PA1.

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)                                                // Ejecuta la desinicialización únicamente para el periférico ADC1.
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC12_CLK_DISABLE();                                             // Deshabilita el reloj del bloque ADC12.

    /**ADC1 GPIO Configuration
    PA0     ------> ADC1_IN1
    PA1     ------> ADC1_IN2
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0|GPIO_PIN_1);                             // Devuelve PA0 y PA1 a su estado de desinicialización.

  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
