/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usb.c
  * @brief   This file provides code for the configuration
  *          of the USB instances.
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
#include "usb.h"                                                 // Incluye las definiciones y funciones HAL asociadas al periférico USB.

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

PCD_HandleTypeDef hpcd_USB_FS;                                   // Estructura de control utilizada por la HAL para gestionar el dispositivo USB Full Speed.

/* USB init function */

void MX_USB_PCD_Init(void)
{

  /* USER CODE BEGIN USB_Init 0 */

  /* USER CODE END USB_Init 0 */

  /* USER CODE BEGIN USB_Init 1 */

  /* USER CODE END USB_Init 1 */
  hpcd_USB_FS.Instance = USB;                                    // Selecciona la instancia USB del microcontrolador.
  hpcd_USB_FS.Init.dev_endpoints = 8;                            // Configura el número máximo de endpoints disponibles para el dispositivo USB.
  hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;                       // Configura el periférico para trabajar en modo USB Full Speed.
  hpcd_USB_FS.Init.phy_itface = PCD_PHY_EMBEDDED;                // Selecciona la interfaz física USB integrada en el microcontrolador.
  hpcd_USB_FS.Init.Sof_enable = DISABLE;                         // Deshabilita la generación de eventos Start Of Frame.
  hpcd_USB_FS.Init.low_power_enable = DISABLE;                   // Deshabilita el modo de bajo consumo asociado al periférico USB.
  hpcd_USB_FS.Init.lpm_enable = DISABLE;                         // Deshabilita la gestión Link Power Management del enlace USB.
  hpcd_USB_FS.Init.battery_charging_enable = DISABLE;            // Deshabilita la función de detección y gestión de carga de batería por USB.
  if (HAL_PCD_Init(&hpcd_USB_FS) != HAL_OK)                      // Inicializa el controlador USB y comprueba posibles errores.
  {
    Error_Handler();                                             // Ejecuta el gestor de error si falla la operación anterior.
  }
  /* USER CODE BEGIN USB_Init 2 */

  /* USER CODE END USB_Init 2 */

}

void HAL_PCD_MspInit(PCD_HandleTypeDef* pcdHandle)
{

  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};                  // Estructura utilizada para configurar la fuente de reloj del periférico USB.
  if(pcdHandle->Instance==USB)                                   // Comprueba que la instancia gestionada corresponde al periférico USB.
  {
  /* USER CODE BEGIN USB_MspInit 0 */

  /* USER CODE END USB_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;      // Selecciona la configuración del reloj correspondiente al periférico USB.
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;    // Utiliza el oscilador interno HSI48 como fuente de reloj de 48 MHz para USB.
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)     // Aplica la configuración del reloj USB y comprueba posibles errores.
    {
      Error_Handler();                                           // Ejecuta el gestor de error si falla la operación anterior.
    }

    /* USB clock enable */
    __HAL_RCC_USB_CLK_ENABLE();                                  // Habilita el reloj del periférico USB.
  /* USER CODE BEGIN USB_MspInit 1 */

  /* USER CODE END USB_MspInit 1 */
  }
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef* pcdHandle)
{

  if(pcdHandle->Instance==USB)                                   // Comprueba que la instancia gestionada corresponde al periférico USB.
  {
  /* USER CODE BEGIN USB_MspDeInit 0 */

  /* USER CODE END USB_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USB_CLK_DISABLE();                                 // Deshabilita el reloj del periférico USB.
  /* USER CODE BEGIN USB_MspDeInit 1 */

  /* USER CODE END USB_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

