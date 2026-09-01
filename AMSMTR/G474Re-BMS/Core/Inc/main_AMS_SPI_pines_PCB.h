/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* -------------------------------------------------------------------------- */
/* Peripheral aliases                                                         */
/* -------------------------------------------------------------------------- */

#define SPI_DAISY                              hspi1
#define UART_HANDLE                            hlpuart1
#define FDCAN_HANDLE                           hfdcan1

/* -------------------------------------------------------------------------- */
/* ADC                                                                        */
/* -------------------------------------------------------------------------- */

#define ADC1_IN1_Pin                           GPIO_PIN_0
#define ADC1_IN1_GPIO_Port                     GPIOA

#define ADC1_IN2_Pin                           GPIO_PIN_1
#define ADC1_IN2_GPIO_Port                     GPIOA

/* -------------------------------------------------------------------------- */
/* DAC                                                                        */
/* -------------------------------------------------------------------------- */

#define DAC1_OUT1_Pin                          GPIO_PIN_4
#define DAC1_OUT1_GPIO_Port                    GPIOA

/* -------------------------------------------------------------------------- */
/* ADBMS                                                                      */
/* -------------------------------------------------------------------------- */

#define ADBMS_INTR1_IN_Pin                     GPIO_PIN_9
#define ADBMS_INTR1_IN_GPIO_Port               GPIOA

#define ADBMS_WAKE_OUT_Pin                     GPIO_PIN_10
#define ADBMS_WAKE_OUT_GPIO_Port               GPIOA

#define ADBMS_INTR2_IN_Pin                     GPIO_PIN_0
#define ADBMS_INTR2_IN_GPIO_Port               GPIOC

#define ADBMS_WAKE2_OUT_Pin                    GPIO_PIN_1
#define ADBMS_WAKE2_OUT_GPIO_Port              GPIOC

/* -------------------------------------------------------------------------- */
/* SPI1                                                                       */
/* -------------------------------------------------------------------------- */

#define SPI1_SCK_Pin                           GPIO_PIN_5
#define SPI1_SCK_GPIO_Port                     GPIOA

#define SPI1_MISO_Pin                          GPIO_PIN_4
#define SPI1_MISO_GPIO_Port                    GPIOB

#define SPI1_MOSI_Pin                          GPIO_PIN_5
#define SPI1_MOSI_GPIO_Port                    GPIOB

#define SPI1_CS_OUT_Pin                        GPIO_PIN_8
#define SPI1_CS_OUT_GPIO_Port                  GPIOA

/* Compatibility with existing BMS code */
#define SPI_DAISY_NSS_Pin                      SPI1_CS_OUT_Pin
#define SPI_DAISY_NSS_GPIO_Port                SPI1_CS_OUT_GPIO_Port

/* -------------------------------------------------------------------------- */
/* SPI2                                                                       */
/* -------------------------------------------------------------------------- */

#define SPI2_SCK_Pin                           GPIO_PIN_1
#define SPI2_SCK_GPIO_Port                     GPIOF

#define SPI2_MISO_Pin                          GPIO_PIN_14
#define SPI2_MISO_GPIO_Port                    GPIOB

#define SPI2_MOSI_Pin                          GPIO_PIN_15
#define SPI2_MOSI_GPIO_Port                    GPIOB

#define SPI2_CS_OUT_Pin                        GPIO_PIN_13
#define SPI2_CS_OUT_GPIO_Port                  GPIOC

/* -------------------------------------------------------------------------- */
/* SPI3                                                                       */
/* -------------------------------------------------------------------------- */

#define SPI3_SCK_Pin                           GPIO_PIN_10
#define SPI3_SCK_GPIO_Port                     GPIOC

#define SPI3_MISO_Pin                          GPIO_PIN_11
#define SPI3_MISO_GPIO_Port                    GPIOC

#define SPI3_MOSI_Pin                          GPIO_PIN_12
#define SPI3_MOSI_GPIO_Port                    GPIOC

#define SPI3_CS_OUT_Pin                        GPIO_PIN_9
#define SPI3_CS_OUT_GPIO_Port                  GPIOC

/* -------------------------------------------------------------------------- */
/* FDCAN1                                                                     */
/* -------------------------------------------------------------------------- */

#define FDCAN1_TX_Pin                          GPIO_PIN_9
#define FDCAN1_TX_GPIO_Port                    GPIOB

#define FDCAN1_RX_Pin                          GPIO_PIN_8
#define FDCAN1_RX_GPIO_Port                    GPIOB

/* -------------------------------------------------------------------------- */
/* FDCAN2                                                                     */
/* -------------------------------------------------------------------------- */

#define FDCAN2_TX_Pin                          GPIO_PIN_13
#define FDCAN2_TX_GPIO_Port                    GPIOB

#define FDCAN2_RX_Pin                          GPIO_PIN_12
#define FDCAN2_RX_GPIO_Port                    GPIOB

/* -------------------------------------------------------------------------- */
/* CAN transceiver control                                                    */
/* -------------------------------------------------------------------------- */

#define CAN_STB1_OUT_Pin                       GPIO_PIN_7
#define CAN_STB1_OUT_GPIO_Port                 GPIOB

#define CAN_STB2_OUT_Pin                       GPIO_PIN_6
#define CAN_STB2_OUT_GPIO_Port                 GPIOB

/* -------------------------------------------------------------------------- */
/* Accelerometer control                                                      */
/* -------------------------------------------------------------------------- */

#define ACCLR_INT_IN_Pin                       GPIO_PIN_2
#define ACCLR_INT_IN_GPIO_Port                 GPIOC

#define ACCLR_BOOTN_OUT_Pin                    GPIO_PIN_3
#define ACCLR_BOOTN_OUT_GPIO_Port              GPIOC

#define ACCLR_CLKSEL1_OUT_Pin                  GPIO_PIN_4
#define ACCLR_CLKSEL1_OUT_GPIO_Port            GPIOC

#define ACCLR_CLKSEL2_OUT_Pin                  GPIO_PIN_5
#define ACCLR_CLKSEL2_OUT_GPIO_Port            GPIOC

#define ACCLR_PS0_OUT_Pin                      GPIO_PIN_6
#define ACCLR_PS0_OUT_GPIO_Port                GPIOC

#define ACCLR_PS1_OUT_Pin                      GPIO_PIN_7
#define ACCLR_PS1_OUT_GPIO_Port                GPIOC

#define ACCLR_RST_OUT_Pin                      GPIO_PIN_8
#define ACCLR_RST_OUT_GPIO_Port                GPIOC

/* -------------------------------------------------------------------------- */
/* QuadSPI                                                                    */
/* -------------------------------------------------------------------------- */

#define QUADSPI1_BK1_IO0_Pin                   GPIO_PIN_1
#define QUADSPI1_BK1_IO0_GPIO_Port             GPIOB

#define QUADSPI1_BK1_IO1_Pin                   GPIO_PIN_0
#define QUADSPI1_BK1_IO1_GPIO_Port             GPIOB

#define QUADSPI1_BK1_IO2_Pin                   GPIO_PIN_7
#define QUADSPI1_BK1_IO2_GPIO_Port             GPIOA

#define QUADSPI1_BK1_IO3_Pin                   GPIO_PIN_6
#define QUADSPI1_BK1_IO3_GPIO_Port             GPIOA

#define QUADSPI1_CLK_Pin                       GPIO_PIN_10
#define QUADSPI1_CLK_GPIO_Port                 GPIOB

#define QUADSPI1_BK1_NCS_Pin                   GPIO_PIN_11
#define QUADSPI1_BK1_NCS_GPIO_Port             GPIOB

/* -------------------------------------------------------------------------- */
/* USB                                                                        */
/* -------------------------------------------------------------------------- */

#define USB_DM_Pin                             GPIO_PIN_11
#define USB_DM_GPIO_Port                       GPIOA

#define USB_DP_Pin                             GPIO_PIN_12
#define USB_DP_GPIO_Port                       GPIOA

/* -------------------------------------------------------------------------- */
/* Auxiliary digital output                                                   */
/* -------------------------------------------------------------------------- */

#define LATCH_OUT_Pin                          GPIO_PIN_2
#define LATCH_OUT_GPIO_Port                    GPIOA

/* -------------------------------------------------------------------------- */
/* SWD debug interface                                                        */
/* -------------------------------------------------------------------------- */

#define T_SWDIO_Pin                            GPIO_PIN_13
#define T_SWDIO_GPIO_Port                      GPIOA

#define T_SWCLK_Pin                            GPIO_PIN_14
#define T_SWCLK_GPIO_Port                      GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */