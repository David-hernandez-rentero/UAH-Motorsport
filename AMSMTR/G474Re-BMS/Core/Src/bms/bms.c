#include "bms/bms.h"

#include "bms/adbms6830b.h"
#include "bms/can.h"
#include "bms/config.h"
#include "stm32g4xx_hal.h"

#include <stdint.h>
#include <stdio.h>

/**
 * Saturating sub
 *
 * <a href="https://locklessinc.com/articles/sat_arithmetic/">Source</a>
 */
uint32_t saturating_sub(uint32_t x, uint32_t y);
inline uint32_t saturating_sub(uint32_t const x, uint32_t const y) {
  uint32_t res = x - y;
  res &= -(res <= x);
  return res;
}

#define HANDLE_ERR(err, msg)                                                   \
  do {                                                                         \
    TRACE("### Error 0x%02X " msg ". Waiting " /* */                           \
          STRINGIFY_VALUE(BMS_ERR_DELAY_MILLIS) "ms and retrying...\n",        \
          err);                                                                \
    HAL_Delay(BMS_ERR_DELAY_MILLIS);                                           \
  } while (0)

void bms_setup(void) {
  printf(
      // clang-format off
      "###################################\n"
      "#\tBMS monitoring started\n"
      "#\tVersion: " STRINGIFY_VALUE(GIT_HASH) "\n"
      "#\tConfig:\n"
      "#\t\t" STRINGIFY_KEY(DATA_IN_USB) "          = " STRINGIFY_VALUE(DATA_IN_USB)          "\n"
      "#\t\t" STRINGIFY_KEY(PRINTF_IN_USB) "        = " STRINGIFY_VALUE(PRINTF_IN_USB)        "\n"
      "#\t\t" STRINGIFY_KEY(DATA_IN_CAN) "          = " STRINGIFY_VALUE(DATA_IN_CAN)          "\n"
      "#\t\t" STRINGIFY_KEY(NUM_DAISY_ADBMS) "      = " STRINGIFY_VALUE(NUM_DAISY_ADBMS)      "\n"
      "#\t\t" STRINGIFY_KEY(NUM_MUX_CHANNELS) "     = " STRINGIFY_VALUE(NUM_MUX_CHANNELS)     "\n"
      "#\t\t" STRINGIFY_KEY(NUM_MUX_EXCEPT_LAST) "  = " STRINGIFY_VALUE(NUM_MUX_EXCEPT_LAST)  "\n"
      "#\t\t" STRINGIFY_KEY(NUM_MUX_LAST) "         = " STRINGIFY_VALUE(NUM_MUX_LAST)         "\n"
      "#\t\t" STRINGIFY_KEY(OVERCURRENT_VOLTS) "    = " STRINGIFY_VALUE(OVERCURRENT_VOLTS)    "\n"
      "#\t\t" STRINGIFY_KEY(BMS_LOOP_MILLIS) "      = " STRINGIFY_VALUE(BMS_LOOP_MILLIS)      "\n"
      "#\t\t" STRINGIFY_KEY(BMS_ERR_DELAY_MILLIS) " = " STRINGIFY_VALUE(BMS_ERR_DELAY_MILLIS) "\n"
      "###################################\n"
      // clang-format on
  );

  TRACE("-> [BEGIN] BMS Setup\n");
  int err;
  while ((err = adbms6830b_setup()))
    HANDLE_ERR(err, "while (re-)configuring the ADBMS6830B");
#if DATA_IN_CAN
  while ((err = can_setup()))
    HANDLE_ERR(err, "while (re-)configuring the CAN bus");
#endif
  TRACE("<- [ END ] BMS Setup\n");
}

void bms_loop(void) {
  TRACE("-> [BEGIN] BMS Loop\n");
  uint32_t const tick_start = HAL_GetTick();
  {
    int err;
    while ((err = adbms6830b_loop())) {
      HANDLE_ERR(err, "in the BMS loop");
      bms_setup();
    }
  }
  HAL_Delay(saturating_sub(BMS_LOOP_MILLIS, HAL_GetTick() - tick_start));
  TRACE("<- [ END ] BMS Loop\n");
}

MUST_USE bool bms_is_overcurrent(void) {
  // TODO
  return false;
}
