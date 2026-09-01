/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    usart.c
 * @brief   This file provides code for the configuration
 *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "main.h"
#include "stm32g4xx_hal_def.h"
#include "stm32g4xx_hal_gpio.h"
#include "stm32g4xx_hal_uart.h"

#include <stdbool.h>
#include <stdlib.h>
/* USER CODE END 0 */

UART_HandleTypeDef hlpuart1;                                             // Global HAL handle used to manage the LPUART1 peripheral

/* LPUART1 init function */
void MX_LPUART1_UART_Init(void) {

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */

  hlpuart1.Instance = LPUART1;                                           // Selects the LPUART1 peripheral
  hlpuart1.Init.BaudRate = 115200;                                       // Sets the communication speed to 115200 bit/s
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;                         // Configures an 8-bit data word
  hlpuart1.Init.StopBits = UART_STOPBITS_1;                              // Uses one stop bit
  hlpuart1.Init.Parity = UART_PARITY_NONE;                               // Disables parity checking
  hlpuart1.Init.Mode = UART_MODE_TX_RX;                                  // Enables both transmission and reception
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;                         // Disables RTS/CTS hardware flow control
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;            // Disables one-bit sampling mode
  hlpuart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;                    // Uses the LPUART peripheral clock without division
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;        // Leaves advanced UART features at their default values

  if (HAL_UART_Init(&hlpuart1) != HAL_OK) {                              // Initializes LPUART1 using the configured parameters
    Error_Handler();                                                     // Stops execution if the peripheral initialization fails
  }

  if (HAL_UARTEx_SetTxFifoThreshold(&hlpuart1, UART_TXFIFO_THRESHOLD_1_8) !=
      HAL_OK) {                                                          // Sets the transmission FIFO threshold to 1/8
    Error_Handler();                                                     // Handles an error while configuring the TX FIFO threshold
  }

  if (HAL_UARTEx_SetRxFifoThreshold(&hlpuart1, UART_RXFIFO_THRESHOLD_1_8) !=
      HAL_OK) {                                                          // Sets the reception FIFO threshold to 1/8
    Error_Handler();                                                     // Handles an error while configuring the RX FIFO threshold
  }

  if (HAL_UARTEx_DisableFifoMode(&hlpuart1) != HAL_OK) {                 // Disables FIFO mode so LPUART1 operates in standard mode
    Error_Handler();                                                     // Handles an error while disabling FIFO mode
  }

  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */
}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle) {

  GPIO_InitTypeDef GPIO_InitStruct = {0};                                // Structure used to configure the LPUART1 GPIO pins
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};                          // Structure used to select the LPUART1 clock source

  if (uartHandle->Instance == LPUART1) {                                 // Executes this configuration only for LPUART1

    /* USER CODE BEGIN LPUART1_MspInit 0 */

    /* USER CODE END LPUART1_MspInit 0 */

    /** Initializes the peripherals clocks */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;          // Selects LPUART1 for peripheral clock configuration
    PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;    // Uses APB1 peripheral clock as the LPUART1 clock source

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {           // Applies the selected LPUART1 clock configuration
      Error_Handler();                                                   // Stops execution if the clock configuration fails
    }

    __HAL_RCC_LPUART1_CLK_ENABLE();                                      // Enables the LPUART1 peripheral clock
    __HAL_RCC_GPIOA_CLK_ENABLE();                                        // Enables GPIOA clock for the TX and RX pins

    /**LPUART1 GPIO Configuration
    PA2     ------> LPUART1_TX
    PA3     ------> LPUART1_RX
    */

    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;                       // Selects PA2 (TX) and PA3 (RX)
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;                              // Configures both pins as alternate-function push-pull
    GPIO_InitStruct.Pull = GPIO_NOPULL;                                  // Disables internal pull-up and pull-down resistors
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                         // Uses low GPIO switching speed
    GPIO_InitStruct.Alternate = GPIO_AF12_LPUART1;                       // Assigns the LPUART1 alternate function to PA2 and PA3
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                              // Applies the GPIO configuration to port A

    /* USER CODE BEGIN LPUART1_MspInit 1 */

    /* USER CODE END LPUART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle) {

  if (uartHandle->Instance == LPUART1) {                                 // Executes the deinitialization only for LPUART1

    /* USER CODE BEGIN LPUART1_MspDeInit 0 */

    /* USER CODE END LPUART1_MspDeInit 0 */

    __HAL_RCC_LPUART1_CLK_DISABLE();                                     // Disables the LPUART1 peripheral clock

    /**LPUART1 GPIO Configuration
    PA2     ------> LPUART1_TX
    PA3     ------> LPUART1_RX
    */

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);                     // Releases PA2 and PA3 from the LPUART1 alternate function

    /* USER CODE BEGIN LPUART1_MspDeInit 1 */

    /* USER CODE END LPUART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

uint8_t uart_block_tx(uint8_t const *Buf, size_t Len) {

  uint8_t result = HAL_OK;                                               // Stores the status returned by HAL_UART_Transmit()

  while (Len) {                                                          // Repeats until the complete buffer has been transmitted

    uint16_t u16_len = Len > 65535 ? 65535 : Len;                        // Limits each transfer to the maximum uint16_t HAL length

    while ((result = HAL_UART_Transmit(&UART_HANDLE, Buf, u16_len,
                                       HAL_MAX_DELAY)) == HAL_BUSY) {    // Retries the blocking transmission while the UART is busy
    }

    if (result)                                                          // Checks whether the transmission returned an error status
      return result;                                                     // Stops and returns the HAL error code

    Buf += u16_len;                                                      // Advances the buffer pointer by the transmitted byte count
    Len -= u16_len;                                                      // Removes the transmitted bytes from the remaining length
  }

  return result;                                                         // Returns HAL_OK after transmitting the complete buffer
}

/* USER CODE END 1 */
