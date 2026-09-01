/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    spi.c
  * @brief   This file provides code for the configuration
  *          of the SPI instances.
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
#include "spi.h"                                                    // Incluye las definiciones y funciones HAL asociadas a los periféricos SPI.

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

SPI_HandleTypeDef hspi1;                                            // Estructura de control utilizada por la HAL para gestionar SPI1.
SPI_HandleTypeDef hspi2;                                            // Estructura de control utilizada por la HAL para gestionar SPI2.
SPI_HandleTypeDef hspi3;                                            // Estructura de control utilizada por la HAL para gestionar SPI3.

/* SPI1 init function */
void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  hspi1.Instance = SPI1;                                            // Selecciona la instancia SPI1 del microcontrolador.
  hspi1.Init.Mode = SPI_MODE_MASTER;                                // Configura SPI1 para funcionar como maestro.
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;                      // Habilita comunicación bidireccional mediante las líneas MISO y MOSI.
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;                          // Configura palabras de datos de 8 bits.
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;                        // Mantiene el reloj a nivel bajo en estado de reposo.
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;                            // Realiza el muestreo de datos en el primer flanco del reloj.
  hspi1.Init.NSS = SPI_NSS_SOFT;                                    // Gestiona por software la señal de selección del esclavo.
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;         // Divide el reloj del periférico entre 128 para fijar la velocidad SPI.
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;                           // Transmite primero el bit más significativo.
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;                           // Deshabilita el modo compatible con el protocolo TI.
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;           // Deshabilita el cálculo hardware de CRC.
  hspi1.Init.CRCPolynomial = 7;                                     // Define el polinomio CRC aunque su cálculo está deshabilitado.
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;                   // Ajusta la longitud del CRC al tamaño de palabra configurado.
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;                       // Habilita el modo de pulso NSS del periférico.
  if (HAL_SPI_Init(&hspi1) != HAL_OK)                               // Inicializa SPI1 con los parámetros configurados y comprueba posibles errores.
  {
    Error_Handler();                                                // Ejecuta el gestor de error si falla la operación anterior.
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}
/* SPI2 init function */
void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  hspi2.Instance = SPI2;                                            // Selecciona la instancia SPI2 del microcontrolador.
  hspi2.Init.Mode = SPI_MODE_MASTER;                                // Configura SPI2 para funcionar como maestro.
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;                      // Habilita comunicación bidireccional mediante las líneas MISO y MOSI.
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;                          // Configura palabras de datos de 8 bits.
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;                        // Mantiene el reloj a nivel bajo en estado de reposo.
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;                            // Realiza el muestreo de datos en el primer flanco del reloj.
  hspi2.Init.NSS = SPI_NSS_SOFT;                                    // Gestiona por software la señal de selección del esclavo.
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;         // Divide el reloj del periférico entre 128 para fijar la velocidad SPI.
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;                           // Transmite primero el bit más significativo.
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;                           // Deshabilita el modo compatible con el protocolo TI.
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;           // Deshabilita el cálculo hardware de CRC.
  hspi2.Init.CRCPolynomial = 7;                                     // Define el polinomio CRC aunque su cálculo está deshabilitado.
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;                   // Ajusta la longitud del CRC al tamaño de palabra configurado.
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;                       // Habilita el modo de pulso NSS del periférico.
  if (HAL_SPI_Init(&hspi2) != HAL_OK)                               // Inicializa SPI2 con los parámetros configurados y comprueba posibles errores.
  {
    Error_Handler();                                                // Ejecuta el gestor de error si falla la operación anterior.
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}
/* SPI3 init function */
void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  hspi3.Instance = SPI3;                                            // Selecciona la instancia SPI3 del microcontrolador.
  hspi3.Init.Mode = SPI_MODE_MASTER;                                // Configura SPI3 para funcionar como maestro.
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;                      // Habilita comunicación bidireccional mediante las líneas MISO y MOSI.
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;                          // Configura palabras de datos de 8 bits.
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;                        // Mantiene el reloj a nivel bajo en estado de reposo.
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;                            // Realiza el muestreo de datos en el primer flanco del reloj.
  hspi3.Init.NSS = SPI_NSS_SOFT;                                    // Gestiona por software la señal de selección del esclavo.
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;         // Divide el reloj del periférico entre 128 para fijar la velocidad SPI.
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;                           // Transmite primero el bit más significativo.
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;                           // Deshabilita el modo compatible con el protocolo TI.
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;           // Deshabilita el cálculo hardware de CRC.
  hspi3.Init.CRCPolynomial = 7;                                     // Define el polinomio CRC aunque su cálculo está deshabilitado.
  hspi3.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;                   // Ajusta la longitud del CRC al tamaño de palabra configurado.
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;                       // Habilita el modo de pulso NSS del periférico.
  if (HAL_SPI_Init(&hspi3) != HAL_OK)                               // Inicializa SPI3 con los parámetros configurados y comprueba posibles errores.
  {
    Error_Handler();                                                // Ejecuta el gestor de error si falla la operación anterior.
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};                           // Estructura utilizada para configurar los GPIO asociados a las interfaces SPI.
  if(spiHandle->Instance==SPI1)                                     // Comprueba si se está inicializando o desinicializando SPI1.
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */
    /* SPI1 clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();                                    // Habilita el reloj del periférico SPI1.

    __HAL_RCC_GPIOA_CLK_ENABLE();                                   // Habilita el reloj del puerto GPIOA.
    __HAL_RCC_GPIOB_CLK_ENABLE();                                   // Habilita el reloj del puerto GPIOB.
    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PB4     ------> SPI1_MISO
    PB5     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5;                               // Selecciona PA5 para la señal de reloj SCK de SPI1.
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;                         // Configura los pines en modo de función alternativa push-pull.
    GPIO_InitStruct.Pull = GPIO_NOPULL;                             // Deshabilita las resistencias internas de pull-up y pull-down.
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                    // Configura una velocidad de conmutación baja para los GPIO.
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;                      // Asigna la función alternativa AF5 correspondiente a SPI1.
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                         // Aplica la configuración definida a los pines seleccionados de GPIOA.

    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;                    // Selecciona PB4 y PB5 para las señales MISO y MOSI de SPI1.
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;                         // Configura los pines en modo de función alternativa push-pull.
    GPIO_InitStruct.Pull = GPIO_NOPULL;                             // Deshabilita las resistencias internas de pull-up y pull-down.
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                    // Configura una velocidad de conmutación baja para los GPIO.
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;                      // Asigna la función alternativa AF5 correspondiente a SPI1.
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);                         // Aplica la configuración definida a los pines seleccionados de GPIOB.

  /* USER CODE BEGIN SPI1_MspInit 1 */

  /* USER CODE END SPI1_MspInit 1 */
  }
  else if(spiHandle->Instance==SPI2)                                // Comprueba si se está inicializando o desinicializando SPI2.
  {
  /* USER CODE BEGIN SPI2_MspInit 0 */

  /* USER CODE END SPI2_MspInit 0 */
    /* SPI2 clock enable */
    __HAL_RCC_SPI2_CLK_ENABLE();                                    // Habilita el reloj del periférico SPI2.

    __HAL_RCC_GPIOF_CLK_ENABLE();                                   // Habilita el reloj del puerto GPIOF.
    __HAL_RCC_GPIOB_CLK_ENABLE();                                   // Habilita el reloj del puerto GPIOB.
    /**SPI2 GPIO Configuration
    PF1-OSC_OUT     ------> SPI2_SCK
    PB14     ------> SPI2_MISO
    PB15     ------> SPI2_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1;                               // Selecciona PF1 para la señal de reloj SCK de SPI2.
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;                         // Configura los pines en modo de función alternativa push-pull.
    GPIO_InitStruct.Pull = GPIO_NOPULL;                             // Deshabilita las resistencias internas de pull-up y pull-down.
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                    // Configura una velocidad de conmutación baja para los GPIO.
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;                      // Asigna la función alternativa AF5 correspondiente a SPI2.
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);                         // Aplica la configuración definida a los pines seleccionados de GPIOF.

    GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;                  // Selecciona PB14 y PB15 para las señales MISO y MOSI de SPI2.
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;                         // Configura los pines en modo de función alternativa push-pull.
    GPIO_InitStruct.Pull = GPIO_NOPULL;                             // Deshabilita las resistencias internas de pull-up y pull-down.
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                    // Configura una velocidad de conmutación baja para los GPIO.
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;                      // Asigna la función alternativa AF5 correspondiente a SPI2.
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);                         // Aplica la configuración definida a los pines seleccionados de GPIOB.

  /* USER CODE BEGIN SPI2_MspInit 1 */

  /* USER CODE END SPI2_MspInit 1 */
  }
  else if(spiHandle->Instance==SPI3)                                // Comprueba si se está inicializando o desinicializando SPI3.
  {
  /* USER CODE BEGIN SPI3_MspInit 0 */

  /* USER CODE END SPI3_MspInit 0 */
    /* SPI3 clock enable */
    __HAL_RCC_SPI3_CLK_ENABLE();                                    // Habilita el reloj del periférico SPI3.

    __HAL_RCC_GPIOC_CLK_ENABLE();                                   // Habilita el reloj del puerto GPIOC.
    /**SPI3 GPIO Configuration
    PC10     ------> SPI3_SCK
    PC11     ------> SPI3_MISO
    PC12     ------> SPI3_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;      // Selecciona PC10, PC11 y PC12 para SCK, MISO y MOSI de SPI3.
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;                         // Configura los pines en modo de función alternativa push-pull.
    GPIO_InitStruct.Pull = GPIO_NOPULL;                             // Deshabilita las resistencias internas de pull-up y pull-down.
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                    // Configura una velocidad de conmutación baja para los GPIO.
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;                      // Asigna la función alternativa AF6 correspondiente a SPI3.
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);                         // Aplica la configuración definida a los pines seleccionados de GPIOC.

  /* USER CODE BEGIN SPI3_MspInit 1 */

  /* USER CODE END SPI3_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI1)                                     // Comprueba si se está inicializando o desinicializando SPI1.
  {
  /* USER CODE BEGIN SPI1_MspDeInit 0 */

  /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();                                   // Deshabilita el reloj del periférico SPI1.

    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PB4     ------> SPI1_MISO
    PB5     ------> SPI1_MOSI
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);                             // Libera la configuración del pin PA5 utilizado por SPI1.

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_4|GPIO_PIN_5);                  // Libera la configuración de PB4 y PB5 utilizados por SPI1.

  /* USER CODE BEGIN SPI1_MspDeInit 1 */

  /* USER CODE END SPI1_MspDeInit 1 */
  }
  else if(spiHandle->Instance==SPI2)                                // Comprueba si se está inicializando o desinicializando SPI2.
  {
  /* USER CODE BEGIN SPI2_MspDeInit 0 */

  /* USER CODE END SPI2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI2_CLK_DISABLE();                                   // Deshabilita el reloj del periférico SPI2.

    /**SPI2 GPIO Configuration
    PF1-OSC_OUT     ------> SPI2_SCK
    PB14     ------> SPI2_MISO
    PB15     ------> SPI2_MOSI
    */
    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_1);                             // Libera la configuración de PF1 utilizado por SPI2.

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_14|GPIO_PIN_15);                // Libera la configuración de PB14 y PB15 utilizados por SPI2.

  /* USER CODE BEGIN SPI2_MspDeInit 1 */

  /* USER CODE END SPI2_MspDeInit 1 */
  }
  else if(spiHandle->Instance==SPI3)                                // Comprueba si se está inicializando o desinicializando SPI3.
  {
  /* USER CODE BEGIN SPI3_MspDeInit 0 */

  /* USER CODE END SPI3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI3_CLK_DISABLE();                                   // Deshabilita el reloj del periférico SPI3.

    /**SPI3 GPIO Configuration
    PC10     ------> SPI3_SCK
    PC11     ------> SPI3_MISO
    PC12     ------> SPI3_MOSI
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12);    // Libera la configuración de PC10, PC11 y PC12 utilizados por SPI3.

  /* USER CODE BEGIN SPI3_MspDeInit 1 */

  /* USER CODE END SPI3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

