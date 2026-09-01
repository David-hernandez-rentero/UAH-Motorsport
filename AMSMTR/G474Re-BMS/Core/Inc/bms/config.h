#ifndef G474RE_BMS_CONFIG_H
#define G474RE_BMS_CONFIG_H

#define PRINTF_IN_USB 1
#define DATA_IN_USB 1
#define DATA_IN_CAN 1

#define CAN_VOLT_MSG_ID 0x100 // TODO temp
#define CAN_TEMP_MSG_ID 0x200 // TODO temp

#define NUM_DAISY_ADBMS 7 // Number of daisy-chained ADBMS6830B
#define NUM_MUX_CHANNELS                                                       \
  8 // Number of multiplexed temperature-channels in a single GPIO
#define NUM_MUX_EXCEPT_LAST                                                    \
  3 // Number of MUXes in a single ADBMS6830B (except the last one)
#define NUM_MUX_LAST 1      // Number of MUXes in the last ADBMS6830B
#define OVERCURRENT_VOLTS 3 // TODO

#define BMS_LOOP_MILLIS 500
#define BMS_ERR_DELAY_MILLIS 150

#endif // G474RE_BMS_CONFIG_H
