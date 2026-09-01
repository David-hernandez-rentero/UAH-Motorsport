#include "bms/can.h"

#include "bms/macros.h"
#include "fdcan.h"
#include "main.h"
#include "stm32g4xx_hal_conf.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

error can_setup(void) {
  TRACE("--> [BEGIN] CAN Setup\n");
  int err = 0;

  TRACE("---> CAN DeInit\n");
  BUBBLE_ERR(err, ERROR_CAN_TX * (HAL_OK != HAL_FDCAN_DeInit(&FDCAN_HANDLE)));
  TRACE("---> CAN Init\n");
  BUBBLE_ERR(err, ERROR_CAN_TX * (HAL_OK != HAL_FDCAN_Init(&FDCAN_HANDLE)));
  TRACE("---> CAN Start\n");
  BUBBLE_ERR(err, ERROR_CAN_TX * (HAL_OK != HAL_FDCAN_Start(&FDCAN_HANDLE)));

  TRACE("<-- [ END ] CAN Setup\n");
  return err;
}

static inline error can_send_raw(uint32_t const id, uint8_t const *const data,
                                 uint32_t const raw_length) {
  FDCAN_TxHeaderTypeDef header = {
      .Identifier = id,
      .IdType = FDCAN_STANDARD_ID,
      .TxFrameType = FDCAN_DATA_FRAME,
      .DataLength = raw_length,
      .ErrorStateIndicator = FDCAN_ESI_PASSIVE,
      .BitRateSwitch = FDCAN_BRS_OFF,
      .FDFormat = FDCAN_CLASSIC_CAN,
      .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
      .MessageMarker = 0,
  };
  return ERROR_CAN_TX * (HAL_OK != HAL_FDCAN_AddMessageToTxFifoQ(
                                       &FDCAN_HANDLE, &header, data));
}

// NOLINTNEXTLINE (bugprone-easily-swappable-parameters)
error can_send(uint32_t const id, uint16_t const data) {
  // do u16->[u8;2] instead of (u8*)&val to avoid depending on cpu endianness
  uint8_t data_bytes[2] = {
      (uint8_t)data,
      (uint8_t)(data >> 8),
  };
  return can_send_raw(id, data_bytes, FDCAN_DLC_BYTES_2);
}
