/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file       fdcan.c
  * @brief      Configuración de los periféricos FDCAN1 y FDCAN2 del AMS.
  *             Hay 2 lineás de CAN porque se necesitan emplear los formatos 
  *             extended para la red del inversor y el formato standard para 
  *             la comunicación con el IMD 
  * @attention  Hay que revisar como es la comunicación del medidor de corriente
  *             para poder conectarlo a su línea de CAN correspondiente.
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
#include "fdcan.h"                                                                                     // Declaración de los periféricos FDCAN y funciones HAL asociadas

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

FDCAN_HandleTypeDef hfdcan1;                                                                           // Estructura de control utilizada por la HAL para gestionar FDCAN1
FDCAN_HandleTypeDef hfdcan2;                                                                           // Estructura de control utilizada por la HAL para gestionar FDCAN2

/* FDCAN1 init function */
#pragma region FDCAN1 init

void MX_FDCAN1_Init(void)
{
  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */

  hfdcan1.Instance = FDCAN1;                                                                           // Selecciona la instancia FDCAN1
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;                                                        // Utiliza el reloj del periférico sin división adicional
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;                                                      // Configura el controlador para trabajar con tramas CAN clásicas
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;                                                               // Activa el modo normal de transmisión y recepción
  hfdcan1.Init.AutoRetransmission = ENABLE;                                                            // Deshabilita la retransmisión automática tras un error
  hfdcan1.Init.TransmitPause = DISABLE;                                                                // Deshabilita las pausas automáticas entre transmisiones
  hfdcan1.Init.ProtocolException = DISABLE;                                                            // Deshabilita el tratamiento de excepciones específico de CAN FD
  hfdcan1.Init.NominalPrescaler = 16;                                                                  // Define el prescaler de la temporización nominal del bus
  hfdcan1.Init.NominalSyncJumpWidth = 1;                                                               // Establece el salto máximo de resincronización en un cuanto temporal
  hfdcan1.Init.NominalTimeSeg1 = 13;                                                                   // Define el primer segmento temporal de la fase nominal
  hfdcan1.Init.NominalTimeSeg2 = 2;                                                                    // Define el segundo segmento temporal de la fase nominal
  hfdcan1.Init.DataPrescaler = 16;                                                                     // Parámetro de fase de datos CAN FD; no interviene en CAN clásico
  hfdcan1.Init.DataSyncJumpWidth = 1;                                                                  // Parámetro de sincronización de fase de datos CAN FD
  hfdcan1.Init.DataTimeSeg1 = 13;                                                                      // Primer segmento temporal de la fase de datos CAN FD
  hfdcan1.Init.DataTimeSeg2 = 2;                                                                       // Segundo segmento temporal de la fase de datos CAN FD
  hfdcan1.Init.StdFiltersNbr = 0;                                                                      // No reserva filtros hardware para identificadores estándar
  hfdcan1.Init.ExtFiltersNbr = 0;                                                                      // No reserva filtros hardware para identificadores extendidos
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;                                              // Configura la cola de transmisión en modo FIFO

  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)                                                              // Aplica la configuración definida al periférico FDCAN1
  {
    Error_Handler();                                                                                   // Ejecuta el gestor de error si la inicialización falla
  }

  /* Aceptar simultáneamente tramas CAN Standard (11 bits) y Extended (29 bits) */
  if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1,
                                   FDCAN_ACCEPT_IN_RX_FIFO0,
                                   FDCAN_ACCEPT_IN_RX_FIFO0,
                                   FDCAN_REJECT_REMOTE,
                                   FDCAN_REJECT_REMOTE) != HAL_OK)
  {
    Error_Handler();
  }

  #pragma endregion

  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */
}

/* FDCAN2 init function */
#pragma region FDCAN2 init

void MX_FDCAN2_Init(void)
{
  /* USER CODE BEGIN FDCAN2_Init 0 */

  /* USER CODE END FDCAN2_Init 0 */

  /* USER CODE BEGIN FDCAN2_Init 1 */

  /* USER CODE END FDCAN2_Init 1 */

  hfdcan2.Instance = FDCAN2;                                                                           // Selecciona la instancia FDCAN2
  hfdcan2.Init.ClockDivider = FDCAN_CLOCK_DIV1;                                                        // Utiliza el reloj del periférico sin división adicional
  hfdcan2.Init.FrameFormat = FDCAN_FRAME_CLASSIC;                                                      // Configura el controlador para trabajar con tramas CAN clásicas
  hfdcan2.Init.Mode = FDCAN_MODE_NORMAL;                                                               // Activa el modo normal de transmisión y recepción
  hfdcan2.Init.AutoRetransmission = ENABLE;                                                            // Deshabilita la retransmisión automática tras un error
  hfdcan2.Init.TransmitPause = DISABLE;                                                                // Deshabilita las pausas automáticas entre transmisiones
  hfdcan2.Init.ProtocolException = DISABLE;                                                            // Deshabilita el tratamiento de excepciones específico de CAN FD
  hfdcan2.Init.NominalPrescaler = 16;                                                                  // Define el prescaler de la temporización nominal del bus
  hfdcan2.Init.NominalSyncJumpWidth = 1;                                                               // Establece el salto máximo de resincronización en un cuanto temporal
  hfdcan2.Init.NominalTimeSeg1 = 13;                                                                   // Define el primer segmento temporal de la fase nominal
  hfdcan2.Init.NominalTimeSeg2 = 2;                                                                    // Define el segundo segmento temporal de la fase nominal
  hfdcan2.Init.DataPrescaler = 16;                                                                     // Parámetro de fase de datos CAN FD; no interviene en CAN clásico
  hfdcan2.Init.DataSyncJumpWidth = 1;                                                                  // Parámetro de sincronización de fase de datos CAN FD
  hfdcan2.Init.DataTimeSeg1 = 13;                                                                      // Primer segmento temporal de la fase de datos CAN FD
  hfdcan2.Init.DataTimeSeg2 = 2;                                                                       // Segundo segmento temporal de la fase de datos CAN FD
  hfdcan2.Init.StdFiltersNbr = 0;                                                                      // No reserva filtros hardware para identificadores estándar
  hfdcan2.Init.ExtFiltersNbr = 0;                                                                      // No reserva filtros hardware para identificadores extendidos
  hfdcan2.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;                                              // Configura la cola de transmisión en modo FIFO

  if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK)                                                              // Aplica la configuración definida al periférico FDCAN2
  {
    Error_Handler();                                                                                   // Ejecuta el gestor de error si la inicialización falla
  }

  /* Aceptar simultáneamente tramas CAN Standard (11 bits) y Extended (29 bits) */
  if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan2,
                                   FDCAN_ACCEPT_IN_RX_FIFO0,
                                   FDCAN_ACCEPT_IN_RX_FIFO0,
                                   FDCAN_REJECT_REMOTE,
                                   FDCAN_REJECT_REMOTE) != HAL_OK)
  {
    Error_Handler();
  }

  #pragma endregion

  /* USER CODE BEGIN FDCAN2_Init 2 */

  /* USER CODE END FDCAN2_Init 2 */
}

static uint32_t HAL_RCC_FDCAN_CLK_ENABLED = 0;                                                         // Contador de instancias que utilizan el reloj común de FDCAN

#pragma region FDCAN MspInit

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* fdcanHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};                                                              // Estructura utilizada para configurar los GPIO de comunicación
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};                                                        // Estructura utilizada para configurar la fuente de reloj de FDCAN

  if (fdcanHandle->Instance == FDCAN1)                                                                 // Comprueba si se está inicializando la primera interfaz CAN
  {
    /* USER CODE BEGIN FDCAN1_MspInit 0 */

    /* USER CODE END FDCAN1_MspInit 0 */

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;                                          // Selecciona la configuración de reloj del bloque FDCAN
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;                                      // Utiliza PCLK1 como fuente de reloj del controlador FDCAN

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)                                           // Aplica la configuración de la fuente de reloj
    {
      Error_Handler();                                                                                 // Ejecuta el gestor de error si la configuración falla
    }

    HAL_RCC_FDCAN_CLK_ENABLED++;                                                                       // Incrementa el número de interfaces que utilizan el reloj FDCAN
    if (HAL_RCC_FDCAN_CLK_ENABLED == 1)                                                                // Habilita físicamente el reloj únicamente con la primera instancia
    {
      __HAL_RCC_FDCAN_CLK_ENABLE();                                                                    // Activa el reloj común de los periféricos FDCAN
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();                                                                      // Habilita el reloj del puerto GPIOB

    /* FDCAN1 GPIO Configuration
       PB8-BOOT0     ------> FDCAN1_RX
       PB9           ------> FDCAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;                                                     // Selecciona PB8 como RX y PB9 como TX
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;                                                            // Configura los pines en función alternativa push-pull
    GPIO_InitStruct.Pull = GPIO_NOPULL;                                                                // Deshabilita resistencias internas de polarización
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                                                       // Selecciona una velocidad de conmutación baja para los GPIO
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;                                                       // Asigna la función alternativa AF9 correspondiente a FDCAN1
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);                                                            // Aplica la configuración a los pines del puerto GPIOB

    /* USER CODE BEGIN FDCAN1_MspInit 1 */

    /* USER CODE END FDCAN1_MspInit 1 */
  }
  else if (fdcanHandle->Instance == FDCAN2)                                                            // Comprueba si se está inicializando la segunda interfaz CAN
  {
    /* USER CODE BEGIN FDCAN2_MspInit 0 */

    /* USER CODE END FDCAN2_MspInit 0 */

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;                                          // Selecciona la configuración de reloj del bloque FDCAN
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;                                      // Utiliza PCLK1 como fuente de reloj del controlador FDCAN

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)                                           // Aplica la configuración de la fuente de reloj
    {
      Error_Handler();                                                                                 // Ejecuta el gestor de error si la configuración falla
    }

    HAL_RCC_FDCAN_CLK_ENABLED++;                                                                       // Incrementa el número de interfaces que utilizan el reloj FDCAN
    if (HAL_RCC_FDCAN_CLK_ENABLED == 1)                                                                // Habilita físicamente el reloj únicamente con la primera instancia
    {
      __HAL_RCC_FDCAN_CLK_ENABLE();                                                                    // Activa el reloj común de los periféricos FDCAN
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();                                                                      // Habilita el reloj del puerto GPIOB

    /* FDCAN2 GPIO Configuration
       PB12     ------> FDCAN2_RX
       PB13     ------> FDCAN2_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13;                                                   // Selecciona PB12 como RX y PB13 como TX
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;                                                            // Configura los pines en función alternativa push-pull
    GPIO_InitStruct.Pull = GPIO_NOPULL;                                                                // Deshabilita resistencias internas de polarización
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                                                       // Selecciona una velocidad de conmutación baja para los GPIO
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN2;                                                       // Asigna la función alternativa AF9 correspondiente a FDCAN2
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);                                                            // Aplica la configuración a los pines del puerto GPIOB

    /* USER CODE BEGIN FDCAN2_MspInit 1 */

    /* USER CODE END FDCAN2_MspInit 1 */
  }
}

#pragma endregion
#pragma region MspDeInit

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* fdcanHandle)
{
  if (fdcanHandle->Instance == FDCAN1)                                                                 // Comprueba si se está desinicializando FDCAN1
  {
    /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

    /* USER CODE END FDCAN1_MspDeInit 0 */

    HAL_RCC_FDCAN_CLK_ENABLED--;                                                                       // Reduce el número de interfaces que utilizan el reloj común
    if (HAL_RCC_FDCAN_CLK_ENABLED == 0)                                                                // Deshabilita el reloj únicamente cuando ninguna interfaz lo utiliza
    {
      __HAL_RCC_FDCAN_CLK_DISABLE();                                                                   // Desactiva el reloj común del bloque FDCAN
    }

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);                                                   // Libera los pines PB8 y PB9 asociados a FDCAN1

    /* USER CODE BEGIN FDCAN1_MspDeInit 1 */

    /* USER CODE END FDCAN1_MspDeInit 1 */
  }
  else if (fdcanHandle->Instance == FDCAN2)                                                            // Comprueba si se está desinicializando FDCAN2
  {
    /* USER CODE BEGIN FDCAN2_MspDeInit 0 */

    /* USER CODE END FDCAN2_MspDeInit 0 */

    HAL_RCC_FDCAN_CLK_ENABLED--;                                                                       // Reduce el número de interfaces que utilizan el reloj común
    if (HAL_RCC_FDCAN_CLK_ENABLED == 0)                                                                // Deshabilita el reloj únicamente cuando ninguna interfaz lo utiliza
    {
      __HAL_RCC_FDCAN_CLK_DISABLE();                                                                   // Desactiva el reloj común del bloque FDCAN
    }

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12 | GPIO_PIN_13);                                                 // Libera los pines PB12 y PB13 asociados a FDCAN2

    /* USER CODE BEGIN FDCAN2_MspDeInit 1 */

    /* USER CODE END FDCAN2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
