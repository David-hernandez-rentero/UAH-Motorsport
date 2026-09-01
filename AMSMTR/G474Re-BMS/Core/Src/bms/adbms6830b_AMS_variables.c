#include "bms/adbms6830b.h"

#include "bms/adbms6830b_commands.h"
#include "bms/can.h"
#include "bms/config.h"
#include "bms/pec_10_crc.h"
#include "spi.h"
#include "stm32g4xx_hal_conf.h"
#include "usart.h"

#include <stdbool.h>
#include <float.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void adbms6830b_set_crc10_single(
    uint8_t data_crc[6 /* data */ + 2 /* PEC 10 */]);

/**
 * Should only be used for {@link adbms6830b_wakeup_all_daisy_chain} and
 * {@link adbms6830b_block_cmd}
 *
 * @param enable Chip Select. {@code true} if enabled (before each action).
 */
void adbms6830b_cs(bool enable);

/** 
 * - See `ADBMS6830B > Sleep-like states` in README
 */
void adbms6830b_block_wakeup_all_daisy_chain(void);

typedef struct {
  uint8_t *restrict datarx_crc;
  size_t datarx_crc_daisy_offset;
  error (*check)(uint8_t *);
} adbms6830b_cmd_opt;

/**
 * Send a command and optionally receive and check a result
 * @param cmd_datatx_crc The command to be sent, including its PEC 15 CRC. If
 *                       the command has data, it must have its PEC 10 CRC
 *                       already included as shown in the datasheet's
 *                       `Table 42. Write Data PEC Format`.
 * @param cmd_datatx_len The length of the command, including the PEC(s).
 * @param datarx_crc     The array in which the received data will be stored (if
 *                       not {@code NULL}). It must be able to hold
 *                       {@code 6 + 2} bytes (6 for data and 2 for the PEC 10
 *                       CRC) a total of {@code NUM_DAISY_ADBMS} times, each
 *                       iteration offsetted by {@code datarx_crc_daisy_offset}
 *                       bytes.
 * @param datarx_crc_daisy_offset The offset between the start of two
 *                       6-byte-long data values of two daisy-chained devices.
 * @param check          If not {@code NULL}, a function that checks the
 *                       validity of the received data of each ADBMS6830B.
 */
#define adbms6830b_block_cmd(cmd_datatx_crc, cmd_datatx_len, ...)              \
  __inner__adbms6830b_block_cmd(                                               \
      cmd_datatx_crc, cmd_datatx_len,                                          \
      (adbms6830b_cmd_opt){VARARGS_OR_0(__VA_ARGS__)})
/**
 * See {@link adbms6830b_block_cmd}.
 */
error __inner__adbms6830b_block_cmd(uint8_t const *restrict cmd_datatx_crc,
                                    size_t cmd_datatx_len,
                                    adbms6830b_cmd_opt opt);

#define adbms6830b_block_config(CHANNEL, modification)                         \
  do {                                                                         \
    uint8_t write_config[sizeof(W_Config_##CHANNEL) +                          \
                         NUM_DAISY_ADBMS * (6 /* data */ + 2 /* PEC */)] = {   \
        0};                                                                    \
    uint8_t *config = write_config + sizeof(W_Config_##CHANNEL);               \
                                                                               \
    BUBBLE_ERR(err, adbms6830b_block_cmd(                                      \
                        R_Config_##CHANNEL, sizeof(R_Config_##CHANNEL),        \
                        .datarx_crc = config, .datarx_crc_daisy_offset = 8));  \
    memcpy(write_config, W_Config_##CHANNEL, sizeof(W_Config_##CHANNEL));      \
                                                                               \
    for (int daisy = NUM_DAISY_ADBMS; --daisy >= 0; config += 8) {             \
      modification;                                                            \
      adbms6830b_set_crc10_single(config);                                     \
    }                                                                          \
                                                                               \
    BUBBLE_ERR(err, adbms6830b_block_cmd(write_config, sizeof(write_config))); \
  } while (0)

error adbms6830b_block_poll(uint8_t const *restrict poll_cmd_crc);

static uint8_t command_counter = 0;

/* AMS measurement storage is defined in main.c so the complete battery state can be inspected from one place. */
extern volatile float    AMS_cell_voltage[NUM_DAISY_ADBMS * 16U];                                                       // Individual cell voltages in volts
extern volatile float    AMS_battery_voltage;                                                                           // Sum of all cell voltages in volts
extern volatile float    AMS_voltage_min;                                                                               // Minimum individual cell voltage in volts
extern volatile float    AMS_voltage_max;                                                                               // Maximum individual cell voltage in volts
extern volatile bool     AMS_voltage_valid;                                                                             // Indicates that the latest full cell-voltage acquisition is valid
extern volatile uint32_t AMS_last_voltage_tick;                                                                         // Tick of the latest valid cell-voltage acquisition

extern volatile float    AMS_aux_voltage[((NUM_DAISY_ADBMS - 1U) * NUM_MUX_CHANNELS * NUM_MUX_EXCEPT_LAST) +
                                         (NUM_MUX_CHANNELS * NUM_MUX_LAST)];                                            // Physical AUX voltages used by the temperature-sensing channels
extern volatile float    AMS_temperature[((NUM_DAISY_ADBMS - 1U) * NUM_MUX_CHANNELS * NUM_MUX_EXCEPT_LAST) +
                                         (NUM_MUX_CHANNELS * NUM_MUX_LAST)];                                            // Converted battery temperatures in degrees Celsius
extern volatile float    AMS_temperature_min;                                                                           // Minimum converted battery temperature in degrees Celsius
extern volatile float    AMS_temperature_max;                                                                           // Maximum converted battery temperature in degrees Celsius
extern volatile bool     AMS_temperature_valid;                                                                         // True only when every temperature conversion is valid
extern volatile uint32_t AMS_last_temperature_tick;                                                                     // Tick of the latest valid temperature acquisition

static int16_t adbms6830b_read_le_i16(const uint8_t *data);                                                             // Reconstructs one signed 16-bit ADBMS result from low-byte/high-byte register order
static float adbms6830b_raw_to_physical_voltage(const uint8_t *data);                                                   // Converts CxV/GxV raw data using 150 uV/LSB and the 1.5 V offset
static void ams_store_cell_voltages(const uint8_t *data);                                                               // Stores every cell voltage and calculates pack/minimum/maximum voltage
static void ams_store_temperature_inputs(const uint8_t *data);                                                          // Stores every AUX channel and updates temperatures when conversion is available
static bool ams_aux_voltage_to_temperature(float aux_voltage_v, float *temperature_c);                                  // Project-specific NTC/XTR111 conversion hook

static int16_t adbms6830b_read_le_i16(const uint8_t *data)                                                              // Reads the ADBMS low-byte-first signed result format
{
  const uint16_t raw_u16 = (uint16_t)data[0] | ((uint16_t)data[1] << 8U);                                               // Byte 0 = bits 7..0; byte 1 = bits 15..8
  return (int16_t)raw_u16;                                                                                              // Preserve the signed two's-complement ADC result
}

static float adbms6830b_raw_to_physical_voltage(const uint8_t *data)                                                    // Converts a CxV/GxV register pair to its physical input voltage
{
  const int16_t raw = adbms6830b_read_le_i16(data);                                                                     // Reconstruct the signed 16-bit ADC result
  return 1.5f + ((float)raw * 0.000150f);                                                                               // ADBMS6830B transfer: physical voltage = 1.5 V + raw * 150 uV
}

static bool ams_aux_voltage_to_temperature(float aux_voltage_v, float *temperature_c)                                   // Converts the external temperature-sensor signal when its real transfer function is known
{
  (void)aux_voltage_v;                                                                                                  // Keep the code warning-free until the actual NTC/XTR111 calibration is provided
  (void)temperature_c;                                                                                                  // Do not fabricate a temperature from an undocumented electrical transfer function

  /* TODO TEMPERATURE SENSOR:
   * Implement here the real AUX-voltage -> temperature conversion of your NTC + MUX + XTR111 chain.
   * Return true only when *temperature_c contains a valid value in degrees Celsius.
   * The ADBMS6830B conversion above gives the AUX input voltage; it does NOT define the external NTC transfer function.
   */
  return false;                                                                                                         // Keep AMS_temperature_valid false until the real sensor conversion is implemented
}

static void ams_store_cell_voltages(const uint8_t *data)                                                                // Converts and stores all 16 cell channels from every daisy-chained ADBMS6830B
{
#if DATA_IN_USB
  size_t data_idx = 4U;                                                                                                 // Skip the four-byte USB timestamp prepended by adbms6830b_loop_voltages()
#else
  size_t data_idx = 0U;                                                                                                 // Voltage payload starts immediately when USB timestamping is disabled
#endif
  size_t cell_idx = 0U;                                                                                                 // Flat index used for the global AMS_cell_voltage[] array
  float battery_voltage = 0.0f;                                                                                         // Running sum of every individual cell voltage
  float voltage_min = FLT_MAX;                                                                                          // Initialize minimum search above every expected cell voltage
  float voltage_max = -FLT_MAX;                                                                                         // Initialize maximum search below every expected cell voltage

  for (size_t daisy = 0U; daisy < (size_t)NUM_DAISY_ADBMS; ++daisy)                                                     // Process each 32-byte block produced for one ADBMS6830B
  {
    for (size_t cell = 0U; cell < 16U; ++cell)                                                                          // Each ADBMS6830B contributes sixteen averaged cell-voltage results
    {
      const float voltage = adbms6830b_raw_to_physical_voltage(&data[data_idx]);                                        // Convert the current two-byte averaged-cell result to volts
      AMS_cell_voltage[cell_idx++] = voltage;                                                                           // Store the individual cell voltage for diagnostics/protection logic
      battery_voltage += voltage;                                                                                       // Add the cell voltage to the complete series-pack voltage

      if (voltage < voltage_min)                                                                                        // Check whether the current cell is the new minimum
      {
        voltage_min = voltage;                                                                                          // Store the lowest individual cell voltage found so far
      }

      if (voltage > voltage_max)                                                                                        // Check whether the current cell is the new maximum
      {
        voltage_max = voltage;                                                                                          // Store the highest individual cell voltage found so far
      }

      data_idx += 2U;                                                                                                   // Advance to the next signed 16-bit cell-voltage result
    }
  }

  AMS_battery_voltage = battery_voltage;                                                                                // Publish the sum of all cell voltages
  AMS_voltage_min = voltage_min;                                                                                        // Publish the minimum individual cell voltage
  AMS_voltage_max = voltage_max;                                                                                        // Publish the maximum individual cell voltage
  AMS_voltage_valid = (cell_idx == ((size_t)NUM_DAISY_ADBMS * 16U));                                                    // Mark the acquisition valid only when every expected cell was processed
  AMS_last_voltage_tick = HAL_GetTick();                                                                                // Timestamp the completed voltage update
}

static void ams_store_temperature_inputs(const uint8_t *data)                                                           // Converts compacted R_AUX_C data into AUX voltages and, when configured, temperatures
{
#if DATA_IN_USB
  size_t data_idx = 4U;                                                                                                 // Skip the four-byte USB timestamp prepended by adbms6830b_loop_temperatures()
#else
  size_t data_idx = 0U;                                                                                                 // AUX payload starts immediately when USB timestamping is disabled
#endif
  size_t temperature_idx = 0U;                                                                                          // Flat destination index for AUX-voltage and temperature arrays
  float temperature_min = FLT_MAX;                                                                                      // Initialize the minimum-temperature search
  float temperature_max = -FLT_MAX;                                                                                     // Initialize the maximum-temperature search
  bool all_temperatures_valid = true;                                                                                   // Becomes false if any channel cannot be converted to degrees Celsius

  for (size_t daisy = 0U; daisy < (size_t)NUM_DAISY_ADBMS; ++daisy)                                                     // Process every compacted ADBMS AUX block in daisy order
  {
    const size_t inputs_per_mux =                                                                                       // Last ADBMS can intentionally have fewer connected MUX outputs than the others
        (daisy == ((size_t)NUM_DAISY_ADBMS - 1U)) ? (size_t)NUM_MUX_LAST : (size_t)NUM_MUX_EXCEPT_LAST;

    for (size_t mux = 0U; mux < (size_t)NUM_MUX_CHANNELS; ++mux)                                                        // Process each selector position of the external analog multiplexers
    {
      for (size_t input = 0U; input < inputs_per_mux; ++input)                                                          // Process each physical MUX output read through GPIO7/GPIO8/GPIO9
      {
        const float aux_voltage = adbms6830b_raw_to_physical_voltage(&data[data_idx]);                                  // Convert the current signed GxV register result to volts
        float temperature_c = 0.0f;                                                                                     // Destination used only if the project-specific temperature conversion succeeds

        AMS_aux_voltage[temperature_idx] = aux_voltage;                                                                 // Always retain the physical AUX voltage even before the NTC transfer function is configured

        if (ams_aux_voltage_to_temperature(aux_voltage, &temperature_c))                                                // Convert to degrees Celsius only through the documented/calibrated sensor function
        {
          AMS_temperature[temperature_idx] = temperature_c;                                                             // Store the physical temperature for this sensor channel

          if (temperature_c < temperature_min)                                                                          // Check whether the current channel is the new minimum temperature
          {
            temperature_min = temperature_c;                                                                            // Store the lowest valid temperature found so far
          }

          if (temperature_c > temperature_max)                                                                          // Check whether the current channel is the new maximum temperature
          {
            temperature_max = temperature_c;                                                                            // Store the highest valid temperature found so far
          }
        }
        else
        {
          all_temperatures_valid = false;                                                                               // Prevent DBC temperature transmission while any conversion is undefined/invalid
        }

        ++temperature_idx;                                                                                              // Advance to the next global temperature/AUX storage slot
        data_idx += 2U;                                                                                                 // Advance to the next signed 16-bit GxV result
      }
    }
  }

  if (all_temperatures_valid && (temperature_idx > 0U))                                                                 // Publish extrema only after every expected temperature was converted successfully
  {
    AMS_temperature_min = temperature_min;                                                                              // Store the lowest valid battery temperature
    AMS_temperature_max = temperature_max;                                                                              // Store the highest valid battery temperature
    AMS_temperature_valid = true;                                                                                       // Allow main.c to transmit the 0x501B DBC message
    AMS_last_temperature_tick = HAL_GetTick();                                                                          // Timestamp the completed physical-temperature update
  }
  else
  {
    AMS_temperature_valid = false;                                                                                      // Explicitly invalidate temperature extrema while the external transfer function is unavailable
  }
}

/**
 * - See `ADBMS6830B > Setup` in README
 */
error adbms6830b_setup(void) {
  TRACE("--> [BEGIN] ADBMS6830B Setup\n");
  int err = 0;

  // Wakeup
  {
    adbms6830b_block_wakeup_all_daisy_chain();
  }

  // Configure registers
  TRACE("---> [BEGIN] ADBMS6830B Config registers\n");
  {
    BUBBLE_ERR(err, adbms6830b_block_cmd(Reset_Counter, sizeof(Reset_Counter)));
    command_counter = 0;

    // CFGA
    adbms6830b_block_config(A, config[0] |= 0x80 /* Set REFON = 1 */);

    // CFGB
    adbms6830b_block_config(
        B, config[3] = 0xFF /* Set DTMEN = 1, DTRNG = 1, DCTO = u6::MAX */);
  }
  TRACE("<--- [ END ] ADBMS6830B Config registers\n");

  // UNSNAP in case this is not the first time we set up the ADBMS6830B
  BUBBLE_ERR(err, adbms6830b_block_cmd(UNSNAP, sizeof(UNSNAP)));

  // ADC command
  TRACE("---> [BEGIN] ADBMS6830B ADC command\n");
  {
    // Start continuous reading
    BUBBLE_ERR(err, adbms6830b_block_cmd(ADCV_CONT, sizeof(ADCV_CONT)));
    IF_OK(err,
          // Wait at least 8ms so that the avg registers fill up. Each of the 16
          // differential cell inputs are read in 1ms, so just wait until all of
          // them have been processed at least once.
          HAL_Delay(16));
  }
  TRACE("<--- [ END ] ADBMS6830B ADC command\n");

  TRACE("<-- [ END ] ADBMS6830B Setup\n");
  return err;
}

/**
 * See RDACF in the Data Sheet. The last 4 bytes must be filled with 1s
 */
error check_last4bytes_FF(uint8_t *reading) {
  return ERROR_ADBMS_PROTOCOL_VIOLATION *
         ((reading[2] & reading[3] & reading[4] & reading[5]) != 0xFF);
}

/**
 * - See `ADBMS6830B > Loop` in README
 * - See `Nucleo-G474Re > Nucleo-G474Re to PC communication protocol` in README
 */
error adbms6830b_loop_voltages(void) {
  int err = 0;

  // Set length in enum to ensure `data` is not a VLA
  enum {
    readings_per_adbms_len = 16 /* readings */ * 2 /* 16 bit */,
    data_len =
#if DATA_IN_USB
        4 /* timestamp */ +
#endif
        NUM_DAISY_ADBMS * readings_per_adbms_len,
    packet_len = data_len,
  };
  uint8_t data[data_len] = {0};
  size_t data_idx = 0;
  // Nucleo-G474Re > Nucleo-G474Re to PC communication protocol
#if DATA_IN_USB
  TRACE("---> [BEGIN] Get timestamp\n");
  {
    uint32_t timestamp = HAL_GetTick();
    data[data_idx++] = (timestamp >>= 0) & 0x3F;
    data[data_idx++] = (timestamp >>= 6) & 0xFF;
    data[data_idx++] = (timestamp >>= 8) & 0xFF;
    data[data_idx++] = (timestamp >>= 8) & 0xFF;                                                                        // NOLINT (*.DeadStores)
  }
  TRACE("<--- [ END ] Get timestamp\n");
#endif

  // SNAP
  TRACE("---> [BEGIN] ADBMS6830B SNAP\n");
  {
    BUBBLE_ERR(err, adbms6830b_block_cmd(SNAP, sizeof(SNAP)));
  }
  TRACE("<--- [ END ] ADBMS6830B SNAP\n");

  // Read all
  TRACE("---> [BEGIN] ADBMS6830B Read voltages\n");
  {
    uint8_t last_reading[NUM_DAISY_ADBMS * 2 /* data */ +
                         4 /* unused bytes of last reading */ +
                         2 /* PEC 10 */] = {0};

    BUBBLE_ERR(err,
               adbms6830b_block_cmd(R_AvgCell_A, sizeof(R_AvgCell_A),
                                    .datarx_crc = data + (data_idx += 0),
                                    .datarx_crc_daisy_offset = readings_per_adbms_len));
    BUBBLE_ERR(err,
               adbms6830b_block_cmd(R_AvgCell_B, sizeof(R_AvgCell_B),
                                    .datarx_crc = data + (data_idx += 6),
                                    .datarx_crc_daisy_offset = readings_per_adbms_len));
    BUBBLE_ERR(err,
               adbms6830b_block_cmd(R_AvgCell_C, sizeof(R_AvgCell_C),
                                    .datarx_crc = data + (data_idx += 6),
                                    .datarx_crc_daisy_offset = readings_per_adbms_len));
    BUBBLE_ERR(err,
               adbms6830b_block_cmd(R_AvgCell_D, sizeof(R_AvgCell_D),
                                    .datarx_crc = data + (data_idx += 6),
                                    .datarx_crc_daisy_offset = readings_per_adbms_len));
    BUBBLE_ERR(err,
               adbms6830b_block_cmd(R_AvgCell_E, sizeof(R_AvgCell_E),
                                    .datarx_crc = data + (data_idx += 6),
                                    .datarx_crc_daisy_offset = readings_per_adbms_len));
    BUBBLE_ERR(err,
               adbms6830b_block_cmd(R_AvgCell_F, sizeof(R_AvgCell_F),
                                    .datarx_crc = last_reading,
                                    .datarx_crc_daisy_offset = 2 /* data */,
                                    .check = check_last4bytes_FF));

    IF_OK(err, {
      // we do the +=6 from RDACE, so after this we are in RDACF
      data_idx += 6;

      size_t last_reading_idx = 0;
      for (int daisy = NUM_DAISY_ADBMS; --daisy >= 0;) {
        data[data_idx] = last_reading[last_reading_idx++];
        data[data_idx + 1] = last_reading[last_reading_idx++];
        data_idx += readings_per_adbms_len;
      }

      // Compensate for the last offset + RDACF is effectively an u16
      data_idx += -readings_per_adbms_len + 2;
    });
  }
  TRACE("<--- [ END ] ADBMS6830B Read voltages\n");

  // UNSNAP
  TRACE("---> [BEGIN] ADBMS6830B UNSNAP\n");
  {
    // Always try to unsnap, even if there were errors
    err |= adbms6830b_block_cmd(UNSNAP, sizeof(UNSNAP));
  }
  TRACE("<--- [ END ] ADBMS6830B UNSNAP\n");

  if (err == 0)                                                                                                         // Convert/store measurements only after a complete error-free SPI/PEC acquisition
  {
    assert_param(data_idx == packet_len);                                                                               // Verify that the compact voltage payload contains every expected cell reading
    ams_store_cell_voltages(data);                                                                                      // Update all cell voltages plus battery total/minimum/maximum values
  }
  else
  {
    AMS_voltage_valid = false;                                                                                          // Prevent higher-level code from publishing a partial or corrupted voltage acquisition
  }

  // Send data
#if DATA_IN_USB
  TRACE("---> [BEGIN] ADBMS6830B Send raw voltages > UART\n");
  {
    IF_OK(err, {
      assert_param(data_idx == packet_len);
      uart_block_tx(data, packet_len);
    });
  }
  TRACE("<--- [ END ] ADBMS6830B Send raw voltages > UART\n");
#endif
/* The old 0x1234 placeholder CAN transmission has been removed.
   * main.c now publishes Battery_Voltage together with the decoded current using the real DBC message 0x701B. */

  return err;
}

/**
 * - See `ADBMS6830B > Loop` in README
 * - See `Nucleo-G474Re > Nucleo-G474Re to PC communication protocol` in README
 */
error adbms6830b_loop_temperatures(void) {
  int err = 0;

  // Set length in enum to ensure `data` is not a VLA
  enum {
    readings_per_adbms_except_last_len =
        NUM_MUX_CHANNELS * NUM_MUX_EXCEPT_LAST /* readings */ * 2 /* 16 bit */,
    readings_adbms_last_len =
        NUM_MUX_CHANNELS * NUM_MUX_LAST /* readings */ * 2 /* 16 bit */,
    data_len =
#if DATA_IN_USB
        4 /* timestamp */ +
#endif
        NUM_DAISY_ADBMS * readings_per_adbms_except_last_len /* data */ +
        2 /* PEC */,
    packet_len = data_len - 2 /* PEC */
                 - readings_per_adbms_except_last_len + readings_adbms_last_len
  };
  uint8_t data[data_len] = {0};
  size_t data_idx = 0;                                                                                                  // NOLINT (clang-diagnostic-unused-but-set-variable)
  // Nucleo-G474Re > Nucleo-G474Re to PC communication protocol
#if DATA_IN_USB
  TRACE("---> [BEGIN] Get timestamp\n");
  {
    uint32_t timestamp = HAL_GetTick();
    data[data_idx++] = ((timestamp >>= 0) & 0x3F) | 0x40;
    data[data_idx++] = (timestamp >>= 6) & 0xFF;
    data[data_idx++] = (timestamp >>= 8) & 0xFF;
    data[data_idx++] = (timestamp >>= 8) & 0xFF;                                                                        // NOLINT (*.DeadStores)
  }
  TRACE("<--- [ END ] Get timestamp\n");
#endif

  // Read all
  TRACE("---> [BEGIN] ADBMS6830B Read temperatures\n");
  {
    for (int mux = 0; mux < NUM_MUX_CHANNELS; ++mux) {
      // Config GPIOs
      adbms6830b_block_config(A, {
        _Static_assert(CEIL_LOG2(NUM_MUX_CHANNELS) == 3,
                       "The MUXs' selector bits are GPIO1, GPIO2 and GPIO6");
        config[3] = (config[3] & ~0x23) | (mux & 0x03) | ((mux << 3) & 0x20);
      });

      // TODO Compute more precise value
      IF_OK(err, HAL_Delay(10));                                                                                        // Wait t_settle ms
      adbms6830b_block_wakeup_all_daisy_chain();

      BUBBLE_ERR(err, adbms6830b_block_cmd(ADAX, sizeof(ADAX)));
      BUBBLE_ERR(err, adbms6830b_block_poll(PL_AUX));

      _Static_assert(NUM_MUX_EXCEPT_LAST == 3 &&
                         NUM_MUX_LAST <= NUM_MUX_EXCEPT_LAST,
                     "The MUXs' data pins are GPIO7, GPIO8 and GPIO9 (RDAUXC)");
      BUBBLE_ERR(err,
                 adbms6830b_block_cmd(R_AUX_C, sizeof(R_AUX_C),
                                      .datarx_crc = data + data_idx,
                                      .datarx_crc_daisy_offset =
                                          readings_per_adbms_except_last_len));
      data_idx += 6 /* RDAUXC */;
    }
    data_idx +=
        // NOLINT (clang-analyzer-deadcode.DeadStores)
        // -1 bc we have already done +=6 in the for loop
        // -1 bc the last MUX has different amount of MUXs
        (NUM_DAISY_ADBMS - 1 - 1) * readings_per_adbms_except_last_len;

    size_t data_idx_sparse = data_idx;
    for (int mux = NUM_MUX_CHANNELS; --mux >= 0;) {
      for (int last_muxes = NUM_MUX_LAST; --last_muxes >= 0;) {
        data[data_idx++] = data[data_idx_sparse++];
        data[data_idx++] = data[data_idx_sparse++];
      }
      data_idx_sparse += (NUM_MUX_EXCEPT_LAST - NUM_MUX_LAST) * 2 /* u16 */;
    }
  }
  TRACE("<--- [ END ] ADBMS6830B Read temperatures\n");

  if (err == 0)                                                                                                         // Convert/store AUX measurements only after a complete error-free SPI/PEC acquisition
  {
    assert_param(data_idx == packet_len);                                                                               // Verify that the compact payload contains every expected AUX reading
    ams_store_temperature_inputs(data);                                                                                 // Always store AUX voltages and update temperatures when the external conversion is configured
  }
  else
  {
    AMS_temperature_valid = false;                                                                                      // Prevent higher-level code from publishing partial/corrupted temperature data
  }

  // Send data
#if DATA_IN_USB
  TRACE("---> [BEGIN] ADBMS6830B Send raw temperatures > UART\n");
  {
    IF_OK(err, {
      assert_param(data_idx == packet_len);
      uart_block_tx(data, packet_len);
    });
  }
  TRACE("<--- [ END ] ADBMS6830B Send raw temperatures > UART\n");
#endif
/* The old 0x5678 placeholder CAN transmission has been removed.
   * main.c now publishes Temperature_max/Temperature_min through the real DBC message 0x501B once conversion is valid. */

  return err;
}

/**
 * - See `ADBMS6830B > Loop` in README
 * - See `Nucleo-G474Re > Nucleo-G474Re to PC communication protocol` in README
 */
error adbms6830b_loop(void) {
  TRACE("--> [BEGIN] ADBMS6830B Loop\n");
  int err = 0;

  adbms6830b_block_wakeup_all_daisy_chain();
  BUBBLE_ERR(err, adbms6830b_loop_voltages());
  BUBBLE_ERR(err, adbms6830b_loop_temperatures());

  TRACE("<-- [ END ] ADBMS6830B Loop\n");
  return err;
}

inline void
adbms6830b_set_crc10_single(uint8_t data_crc[6 /* data */ + 2 /* PEC 10 */]) {
  data_crc[6] = 0;
  data_crc[7] = 0;
  uint_fast16_t pec10 = pec_10_crc(data_crc, 6, 6);
  assert_param((pec10 & ~0x03FF) == 0);
  data_crc[6] = pec10 >> 8;
  data_crc[7] = pec10;
}

inline void adbms6830b_cs(bool const enable) {
  HAL_GPIO_WritePin(SPI_DAISY_NSS_GPIO_Port, SPI_DAISY_NSS_Pin,
                    enable ? GPIO_PIN_RESET : GPIO_PIN_SET);
  TRACE("-----> ADBMS6830B Chip select %s (%s)\n", enable ? "LOW" : "HIGH",
        enable ? "enabled" : "disabled");
}

inline void adbms6830b_block_wakeup_all_daisy_chain(void) {
  TRACE("---> [BEGIN] ADBMS6830B Wake-up signal\n");
  for (int daisy = NUM_DAISY_ADBMS; --daisy >= 0;) {
    adbms6830b_cs(true);
    HAL_Delay(1);
    adbms6830b_cs(false);
    HAL_Delay(1);
  }
  TRACE("<--- [ END ] ADBMS6830B Wake-up signal\n");
}

error __inner__adbms6830b_block_cmd(
    uint8_t const *restrict const cmd_datatx_crc, size_t const cmd_datatx_len,
    adbms6830b_cmd_opt const opt) {
  TRACE("----> [BEGIN] ADBMS6830B CMD\n");
  int err = 0;

  adbms6830b_cs(true);
  {
    err |= ERROR_SPI_TX * (HAL_OK != HAL_SPI_Transmit(                                                                  //
                                         &SPI_DAISY, cmd_datatx_crc,
                                         cmd_datatx_len, HAL_MAX_DELAY));

    if (opt.datarx_crc == NULL) {
      if (++command_counter >= 64)
        command_counter = 1;
    } else {
      uint8_t *restrict datarx_crc = opt.datarx_crc;
      size_t datarx_crc_daisy_offset = opt.datarx_crc_daisy_offset;

      for (int daisy = NUM_DAISY_ADBMS; --daisy >= 0;) {
        TRACE("-----> [BEGIN] ADBMS6830B rx + Data PEC 10 checking of device "
              "%d/"                                                                                                     //
              STRINGIFY_VALUE(NUM_DAISY_ADBMS) "\n",
              NUM_DAISY_ADBMS - daisy);

        // Save the previous values to avoid overriding them with the PEC 10
        uint8_t const prev_val_in_pec0 = datarx_crc[6];
        uint8_t const prev_val_in_pec1 = datarx_crc[7];

        err |= ERROR_SPI_RX *
               (HAL_OK != HAL_SPI_Receive(                                                                              //
                              &SPI_DAISY, datarx_crc,
                              6 /* data */ + 2 /* PEC 10 Rx */, HAL_MAX_DELAY));
        BUBBLE_ERR(                                                                                                     //
            err, ERROR_CRC_RX *
                     (0 != pec_10_crc(datarx_crc,
                                      6 /* data */ + 2 /* PEC 10 Rx */, 0)));
        BUBBLE_ERR(                                                                                                     //
            err, ERROR_CMD_COUNTER * (command_counter != (datarx_crc[6] >> 2)));

        if (opt.check != NULL)
          BUBBLE_ERR(err, opt.check(datarx_crc));

        // Restore the previous values
        datarx_crc[6] = prev_val_in_pec0;
        datarx_crc[7] = prev_val_in_pec1;

        datarx_crc += datarx_crc_daisy_offset;

        TRACE("<----- [ END ] ADBMS6830B rx + Data PEC 10 checking of device "
              "%d/"                                                                                                     //
              STRINGIFY_VALUE(NUM_DAISY_ADBMS) " (err = %X)\n",
              NUM_DAISY_ADBMS - daisy, err);
      }
    }
  }
  adbms6830b_cs(false);

  TRACE("<---- [ END ] ADBMS6830B CMD\n");
  return err;
}

error adbms6830b_block_poll(uint8_t const *restrict const poll_cmd_crc) {
  TRACE("----> [BEGIN] ADBMS6830B POLL\n");
  int err = 0;

  adbms6830b_cs(true);
  {
    err |= ERROR_SPI_TX * (HAL_OK != HAL_SPI_Transmit(&SPI_DAISY, poll_cmd_crc,
                                                      4, HAL_MAX_DELAY));

    // `The SDO status is valid only at the end of 2 × N clock pulses on SCK`
    enum {
      delay = 1000 /* ms/s */ * (2 * NUM_DAISY_ADBMS) / 2000000 /* 2Mbps */,
    };
    HAL_Delay(delay);
    _Static_assert(delay < 4,
                   "Delay should be at most 4ms, otherwise we are "
                   "forced to call adbms6830b_wakeup_all_daisy_chain");

    uint8_t sdo = 0;
    while (!(err | sdo)) {
      err |= ERROR_SPI_RX *
             (HAL_OK != HAL_SPI_Receive(&SPI_DAISY, &sdo, 1, HAL_MAX_DELAY));
    }
  }
  adbms6830b_cs(false);

  TRACE("<---- [ END ] ADBMS6830B POLL\n");
  return err;
}
