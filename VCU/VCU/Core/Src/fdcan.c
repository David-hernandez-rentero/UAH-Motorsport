/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    fdcan.c
 * @brief   This file provides code for the configuration
 *          of the FDCAN instances.
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
#include "fdcan.h"                                                    // FDCAN peripheral declarations and HAL definitions

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

FDCAN_HandleTypeDef hfdcan1;                                          // Global HAL handle used to control the FDCAN1 peripheral

/* FDCAN1 init function */
void MX_FDCAN1_Init(void)                                             // Configures the FDCAN1 controller, bit timing and receive policy
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;                                          // Selects FDCAN1 as the peripheral controlled by hfdcan1
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;                       // Uses the FDCAN kernel clock without an additional divider
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;                     // Restricts transmission and reception to Classic CAN frames
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;                              // Uses normal bus operation rather than loopback or monitoring mode
  hfdcan1.Init.AutoRetransmission = ENABLE;                           // Automatically retransmits a frame when arbitration or transmission fails
  hfdcan1.Init.TransmitPause = DISABLE;                               // Allows consecutive transmissions without an enforced pause
  hfdcan1.Init.ProtocolException = DISABLE;                           // Disables CAN FD protocol-exception handling
  hfdcan1.Init.NominalPrescaler = 17;                                 // Divides the FDCAN kernel clock for the nominal arbitration bit rate
  hfdcan1.Init.NominalSyncJumpWidth = 1;                              // Sets the nominal resynchronization jump width to one time quantum
  hfdcan1.Init.NominalTimeSeg1 = 12;                                  // Defines nominal timing segment 1 as twelve time quanta
  hfdcan1.Init.NominalTimeSeg2 = 7;                                   // Defines nominal timing segment 2 as seven time quanta
  hfdcan1.Init.DataPrescaler = 17;                                    // Sets the data-phase prescaler; retained even though Classic CAN is used
  hfdcan1.Init.DataSyncJumpWidth = 1;                                 // Sets the data-phase synchronization jump width to one time quantum
  hfdcan1.Init.DataTimeSeg1 = 12;                                     // Defines data-phase timing segment 1 as twelve time quanta
  hfdcan1.Init.DataTimeSeg2 = 7;                                      // Defines data-phase timing segment 2 as seven time quanta
  hfdcan1.Init.StdFiltersNbr = 0;                                     // Reserves no dedicated standard-ID filter elements
  hfdcan1.Init.ExtFiltersNbr = 0;                                     // Reserves no dedicated extended-ID filter elements
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;             // Uses the transmit area as a FIFO instead of a priority queue
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)                             // Initializes FDCAN1 and checks whether the HAL configuration succeeded
  {
    Error_Handler();                                                  // Transfers execution to the global error handler after a failed HAL operation
  }

  /* Accept all standard/extended frames in RX FIFO 0 */
  if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1,
                                   FDCAN_ACCEPT_IN_RX_FIFO0,          // Routes non-matching standard-ID data frames to receive FIFO0
                                   FDCAN_ACCEPT_IN_RX_FIFO0,          // Routes non-matching extended-ID data frames to receive FIFO0
                                   FDCAN_REJECT_REMOTE,               // Rejects non-matching standard remote frames
                                   FDCAN_REJECT_REMOTE) != HAL_OK)    // Rejects non-matching extended remote frames and checks the filter configuration
  {
    Error_Handler();                                                  // Transfers execution to the global error handler after a failed HAL operation
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */
}

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *fdcanHandle)              // Configures clocks, GPIO pins and interrupts required by FDCAN1
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};                             // Initializes the GPIO configuration structure to zero
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};                       // Initializes the peripheral clock configuration structure to zero
  if (fdcanHandle->Instance == FDCAN1)                                // Applies the following low-level configuration only to FDCAN1
  {
    /* USER CODE BEGIN FDCAN1_MspInit 0 */

    /* USER CODE END FDCAN1_MspInit 0 */

    /** Initializes the peripherals clocks
     */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;         // Selects the FDCAN peripheral clock for configuration
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;     // Uses APB1 peripheral clock as the FDCAN kernel clock source
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)          // Applies the FDCAN clock-source configuration and checks for errors
    {
      Error_Handler();                                                // Transfers execution to the global error handler after a failed HAL operation
    }

    /* FDCAN1 clock enable */
    __HAL_RCC_FDCAN_CLK_ENABLE();                                     // Enables the hardware clock supplied to FDCAN1

    __HAL_RCC_GPIOA_CLK_ENABLE();                                     // Enables GPIOA because PA11 and PA12 carry FDCAN1 signals
    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;                  // Selects PA11 as FDCAN1_RX and PA12 as FDCAN1_TX
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;                           // Configures both CAN pins for push-pull alternate-function operation
    GPIO_InitStruct.Pull = GPIO_NOPULL;                               // Disables internal pull resistors on the FDCAN pins
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                      // Uses the configured low GPIO output-speed setting
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;                      // Connects PA11 and PA12 to the FDCAN1 alternate function
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                           // Applies the alternate-function configuration to the FDCAN pins

    /* FDCAN1 interrupt Init */
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 0, 0);                      // Sets FDCAN interrupt line 0 to the highest configured priority
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);                              // Enables FDCAN interrupt line 0 in the NVIC
    HAL_NVIC_SetPriority(FDCAN1_IT1_IRQn, 0, 0);                      // Sets FDCAN interrupt line 1 to the highest configured priority
    HAL_NVIC_EnableIRQ(FDCAN1_IT1_IRQn);                              // Enables FDCAN interrupt line 1 in the NVIC
    /* USER CODE BEGIN FDCAN1_MspInit 1 */

    /* USER CODE END FDCAN1_MspInit 1 */
  }
}

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef *fdcanHandle)            // Releases clocks, GPIO pins and interrupts associated with FDCAN1
{

  if (fdcanHandle->Instance == FDCAN1)                                // Applies the following low-level configuration only to FDCAN1
  {
    /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

    /* USER CODE END FDCAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_FDCAN_CLK_DISABLE();                                    // Disables the hardware clock supplied to FDCAN1

    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);                // Returns the FDCAN pins to their reset configuration

    /* FDCAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);                             // Disables FDCAN interrupt line 0 in the NVIC
    HAL_NVIC_DisableIRQ(FDCAN1_IT1_IRQn);                             // Disables FDCAN interrupt line 1 in the NVIC
    /* USER CODE BEGIN FDCAN1_MspDeInit 1 */

    /* USER CODE END FDCAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
