/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fdcan.h
  * @brief   This file contains all the function prototypes for
  *          the fdcan.c file
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
#ifndef __FDCAN_H__
#define __FDCAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

// ===========================================================================
// VARIABLES DE RECEPCIÓN CAN
// ===========================================================================
extern volatile uint8_t can_ok;
extern uint8_t RxData[8];
extern FDCAN_RxHeaderTypeDef RxHeader;
extern volatile uint32_t can_last_rx_tick;

// Tensiones actualizadas desde la recepción CAN.
extern volatile uint32_t voltaje_inversor_mV;
extern volatile uint32_t voltaje_bateria_mV;

// ===========================================================================
// MENSAJE R2D SEGÚN DBC
// ===========================================================================
#define CAN_ID_R2D                       0x0000301BU
#define CAN_R2D_STATUS_PERIOD_MS         100U

// R2D_RelayStatus - Byte 4
#define R2D_AIR_POS_MASK                 (1U << 0)
#define R2D_AIR_NEG_MASK                 (1U << 1)
#define R2D_PRECHARGE_MASK               (1U << 2)
#define R2D_DISCHARGE_MASK               (1U << 3)
#define R2D_ACTIVATION_MASK              (1U << 4)

// ===========================================================================
// MENSAJE RTD OK LEGACY
// Se mantiene mientras FDCAN_SendRTDOk() siga presente en fdcan.c.
// ===========================================================================
#define CAN_RTD_OK_ID                    0x00003001U
#define CAN_RTD_OK_PERIOD_MS             100U
#define CAN_RTD_OK_VALUE                 0x01U

/* USER CODE END Includes */

extern FDCAN_HandleTypeDef hfdcan1;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_FDCAN1_Init(void);

/* USER CODE BEGIN Prototypes */

void FDCAN_ConfigFilter(void);
void FDCAN_Start(void);
void FDCAN_SendMessage(uint32_t id, uint8_t *data, uint32_t len);
void FDCAN_SendR2DStatus(void);
void FDCAN_SendRTDOk(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __FDCAN_H__ */
