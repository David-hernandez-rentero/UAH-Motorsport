/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body for the AMS/BMS controller.
 * @brief          : Stores battery voltages, temperatures and CAN measurements.
 * @brief          : Sends the AMS information using the vehicle DBC layout.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"                                                                                                       // Main project definitions and Error_Handler declaration
#include "fdcan.h"                                                                                                      // FDCAN1/FDCAN2 handles and CubeMX initialization functions
#include "gpio.h"                                                                                                       // GPIO initialization functions
#include "spi.h"                                                                                                        // SPI initialization used by the ADBMS6830B daisy chain
#include "usart.h"                                                                                                      // LPUART initialization used by TRACE/USB output

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bms/bms.h"                                                                                                    // High-level BMS setup and periodic acquisition loop
#include "bms/can.h"                                                                                                    // CAN error type and legacy CAN interface
#include "bms/config.h"                                                                                                 // Number of ADBMS devices, MUX channels and project options

#include <stdbool.h>                                                                                                    // bool, true and false
#include <stddef.h>                                                                                                     // size_t
#include <stdint.h>                                                                                                     // Fixed-width integer types
#include <stdio.h>                                                                                                      // printf
#include <unistd.h>                                                                                                     // STDOUT_FILENO used by the printf redirection
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

enum
{
  AMS_CELLS_PER_ADBMS = 16U,                                                                                            // Number of cell-voltage channels read from each ADBMS6830B
  AMS_NUM_CELLS = NUM_DAISY_ADBMS * AMS_CELLS_PER_ADBMS,                                                                // Total number of cell voltages stored by the AMS
  AMS_NUM_TEMPERATURES =                                                                                                // Total number of temperature/AUX channels connected to the daisy chain
      ((NUM_DAISY_ADBMS - 1U) * NUM_MUX_CHANNELS * NUM_MUX_EXCEPT_LAST) +
      (NUM_MUX_CHANNELS * NUM_MUX_LAST)
};

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define CAN_ID_AMS_TEMPERATURES          0x0000501BU                                                                    // DBC Extended ID: Temperature_max + Temperature_min
#define CAN_ID_AMS_BATTERY_MEASUREMENTS  0x0000701BU                                                                    // DBC Extended ID: Battery_Current + Battery_Voltage
#define AMS_DBC_TX_PERIOD_MS             100U                                                                           // Minimum period between consecutive AMS DBC transmissions

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#define AMS_DBC_FDCAN_HANDLE             hfdcan1                                                                        // CAN interface used to transmit the AMS DBC messages to the vehicle network

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

volatile float    AMS_cell_voltage[AMS_NUM_CELLS] = {0.0f};                                                             // Individual cell voltages in volts
volatile float    AMS_battery_voltage = 0.0f;                                                                           // Sum of all valid cell voltages in volts
volatile float    AMS_voltage_min = 0.0f;                                                                               // Minimum individual cell voltage in volts
volatile float    AMS_voltage_max = 0.0f;                                                                               // Maximum individual cell voltage in volts
volatile bool     AMS_voltage_valid = false;                                                                            // True when the latest complete voltage acquisition is valid
volatile uint32_t AMS_last_voltage_tick = 0U;                                                                           // Tick of the latest valid voltage acquisition

volatile float    AMS_aux_voltage[AMS_NUM_TEMPERATURES] = {0.0f};                                                       // Raw physical AUX voltages measured for all temperature channels
volatile float    AMS_temperature[AMS_NUM_TEMPERATURES] = {0.0f};                                                       // Temperature values in degrees Celsius after sensor conversion
volatile float    AMS_temperature_min = 0.0f;                                                                           // Minimum battery temperature in degrees Celsius
volatile float    AMS_temperature_max = 0.0f;                                                                           // Maximum battery temperature in degrees Celsius
volatile bool     AMS_temperature_valid = false;                                                                        // True only when every required AUX channel has been converted to degrees Celsius
volatile uint32_t AMS_last_temperature_tick = 0U;                                                                       // Tick of the latest valid temperature acquisition

volatile float    AMS_battery_current = 0.0f;                                                                           // Battery current in amperes after decoding the external CAN current sensor
volatile bool     AMS_current_valid = false;                                                                            // False until the current-sensor CAN protocol is configured in can.c
volatile uint32_t AMS_last_current_tick = 0U;                                                                           // Tick of the latest valid current-sensor message

static uint32_t   AMS_last_dbc_tx_tick = 0U;                                                                            // Tick used to limit the AMS DBC transmission rate

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);                                                                                          // Configures the MCU system clock

/* USER CODE BEGIN PFP */
error             can_setup(void);                                                                                      // Starts FDCAN1/FDCAN2 without destroying their CubeMX filter configuration
void              can_receive_all(void);                                                                                // Drains RX FIFO0 from both FDCAN peripherals and stores every received frame
HAL_StatusTypeDef can_send_frame(FDCAN_HandleTypeDef *hfdcan, uint32_t id, uint32_t id_type,
                                 const uint8_t *data, uint32_t data_length);                                            // Generic Standard/Extended Classic-CAN transmitter
static void       AMS_Send_DBC(void);                                                                                   // Encodes and transmits the AMS messages defined in the current DBC
static void       Write_BE_U32(uint8_t *data, uint32_t value);                                                          // Writes a 32-bit value in Motorola/Big-Endian byte order
static uint32_t   DBC_Encode_Temperature(float temperature_c);                                                          // Encodes Temperature_* using factor 0.1 and offset -10 degrees Celsius
static uint32_t   DBC_Encode_Current(float current_a);                                                                  // Encodes Battery_Current using the current DBC factor of 0.1 A/bit
static uint32_t   DBC_Encode_Voltage(float voltage_v);                                                                  // Encodes Battery_Voltage using the current DBC factor of 0.01 V/bit
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#if PRINTF_IN_USB
extern int _write_weak(int file, char *ptr, int len);                                                                   // Original weak syscall used for streams other than stdout

int _write(int const file, char *const ptr, int const len)                                                              // Redirects printf output to LPUART1/USB
{
  if (file != STDOUT_FILENO)                                                                                            // Preserve the original implementation for non-stdout streams
  {
    return _write_weak(file, ptr, len);                                                                                 // Forward the write operation to the weak syscall
  }

#if DATA_IN_USB
  {
    int encoded_len = len;                                                                                              // Number of payload bytes that will be sent after the LEB128 length field
    size_t i = 0U;                                                                                                      // Current index inside the encoded length buffer
    uint8_t leb128_size[sizeof(encoded_len) + 1U];                                                                      // Temporary storage for the compact LEB128 message length

    leb128_size[i++] = (uint8_t)((encoded_len & 0x3F) | 0x80);                                                          // Store the first six length bits and the framing flag
    encoded_len >>= 6;                                                                                                  // Remove the six bits already encoded

    if (encoded_len != 0)                                                                                               // Encode the remaining length only when more bits are present
    {
      leb128_size[0] |= 0x40U;                                                                                          // Indicate that additional LEB128 bytes follow
      do
      {
        leb128_size[i++] = (uint8_t)((encoded_len & 0x7F) | 0x80);                                                      // Store seven additional bits per LEB128 byte
        encoded_len >>= 7;                                                                                              // Remove the seven bits already encoded
      } while (encoded_len != 0);

      leb128_size[i - 1U] &= 0x7FU;                                                                                     // Clear the continuation flag in the last length byte
    }

    uart_block_tx(leb128_size, i);                                                                                      // Send the encoded message length before the actual printf payload
  }
#endif

  uart_block_tx((uint8_t *)ptr, (size_t)len);                                                                           // Send the printf payload through LPUART1
  return len;                                                                                                           // Report that every requested byte was transmitted
}
#endif

static void Write_BE_U32(uint8_t *data, uint32_t value)                                                                 // Writes one unsigned 32-bit value using DBC Motorola/Big-Endian byte order
{
  data[0] = (uint8_t)(value >> 24U);                                                                                    // Byte 0 contains bits 31..24
  data[1] = (uint8_t)(value >> 16U);                                                                                    // Byte 1 contains bits 23..16
  data[2] = (uint8_t)(value >> 8U);                                                                                     // Byte 2 contains bits 15..8
  data[3] = (uint8_t)value;                                                                                             // Byte 3 contains bits 7..0
}

static uint32_t DBC_Encode_Temperature(float temperature_c)                                                             // Converts degrees Celsius to the raw DBC Temperature_* representation
{
  float raw = (temperature_c + 10.0f) * 10.0f;                                                                          // DBC inverse conversion: raw = (physical - offset) / factor

  if (raw <= 0.0f)                                                                                                      // Protect the unsigned DBC field against values below its representable range
  {
    return 0U;                                                                                                          // Saturate negative raw values to zero
  }

  if (raw >= 4294967295.0f)                                                                                             // Protect the cast against values above the uint32_t range
  {
    return UINT32_MAX;                                                                                                  // Saturate values above the DBC field range
  }

  return (uint32_t)(raw + 0.5f);                                                                                        // Round to the nearest DBC raw integer
}

static uint32_t DBC_Encode_Current(float current_a)                                                                     // Converts amperes to the raw DBC Battery_Current representation
{
  float raw = current_a * 10.0f;                                                                                        // DBC inverse conversion: raw = physical / 0.1 A

  if (raw <= 0.0f)                                                                                                      // Current field is unsigned in the current VCU DBC decoder
  {
    return 0U;                                                                                                          // Saturate negative current until a signed-current DBC definition is provided
  }

  if (raw >= 4294967295.0f)                                                                                             // Protect the cast against values above the uint32_t range
  {
    return UINT32_MAX;                                                                                                  // Saturate values above the DBC field range
  }

  return (uint32_t)(raw + 0.5f);                                                                                        // Round to the nearest DBC raw integer
}

static uint32_t DBC_Encode_Voltage(float voltage_v)                                                                     // Converts volts to the raw DBC Battery_Voltage representation
{
  float raw = voltage_v * 100.0f;                                                                                       // DBC inverse conversion: raw = physical / 0.01 V

  if (raw <= 0.0f)                                                                                                      // Protect the unsigned DBC field against negative values
  {
    return 0U;                                                                                                          // Saturate negative raw values to zero
  }

  if (raw >= 4294967295.0f)                                                                                             // Protect the cast against values above the uint32_t range
  {
    return UINT32_MAX;                                                                                                  // Saturate values above the DBC field range
  }

  return (uint32_t)(raw + 0.5f);                                                                                        // Round to the nearest DBC raw integer
}

static void AMS_Send_DBC(void)                                                                                          // Sends the two AMS messages already expected by the VCU DBC
{
  const uint32_t now = HAL_GetTick();                                                                                   // Read the current system tick once for both transmission decisions

  if ((uint32_t)(now - AMS_last_dbc_tx_tick) < AMS_DBC_TX_PERIOD_MS)                                                    // Keep the CAN transmission rate below the configured period
  {
    return;                                                                                                             // Leave without transmitting when the period has not elapsed
  }

  AMS_last_dbc_tx_tick = now;                                                                                           // Store the transmission time before preparing the messages

  if (AMS_temperature_valid)                                                                                            // Do not publish fabricated temperatures before the NTC conversion is configured
  {
    uint8_t tx_data[8] = {0U};                                                                                          // Eight-byte payload required by message 0x501B

    Write_BE_U32(&tx_data[0], DBC_Encode_Temperature(AMS_temperature_max));                                             // Bytes 0..3: Temperature_max
    Write_BE_U32(&tx_data[4], DBC_Encode_Temperature(AMS_temperature_min));                                             // Bytes 4..7: Temperature_min

    if (can_send_frame(&AMS_DBC_FDCAN_HANDLE, CAN_ID_AMS_TEMPERATURES, FDCAN_EXTENDED_ID,
                       tx_data, FDCAN_DLC_BYTES_8) != HAL_OK)                                                           // Transmit the DBC temperature message as Classic CAN Extended ID
    {
      TRACE("### Error while sending AMS temperature message\n");                                                       // Report the transmission error without inventing measurement data
    }
  }

  if (AMS_voltage_valid && AMS_current_valid)                                                                           // 0x701B is sent only when both signals contained in the frame are valid
  {
    uint8_t tx_data[8] = {0U};                                                                                          // Eight-byte payload required by message 0x701B

    Write_BE_U32(&tx_data[0], DBC_Encode_Current(AMS_battery_current));                                                 // Bytes 0..3: Battery_Current
    Write_BE_U32(&tx_data[4], DBC_Encode_Voltage(AMS_battery_voltage));                                                 // Bytes 4..7: Battery_Voltage

    if (can_send_frame(&AMS_DBC_FDCAN_HANDLE, CAN_ID_AMS_BATTERY_MEASUREMENTS, FDCAN_EXTENDED_ID,
                       tx_data, FDCAN_DLC_BYTES_8) != HAL_OK)                                                           // Transmit the DBC battery measurement message as Classic CAN Extended ID
    {
      TRACE("### Error while sending AMS battery measurement message\n");                                               // Report the transmission error without blocking the BMS acquisition loop
    }
  }
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)                                                                                                          // Main entry point of the AMS firmware
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  HAL_Init();                                                                                                           // Reset peripherals and initialize the STM32 HAL and SysTick
  SystemClock_Config();                                                                                                 // Configure the system clock before initializing peripherals

  MX_GPIO_Init();                                                                                                       // Initialize GPIO used by the AMS board
  MX_SPI1_Init();                                                                                                       // Initialize the SPI port used by the ADBMS6830B/isoSPI chain
  MX_LPUART1_UART_Init();                                                                                               // Initialize the diagnostic/USB serial interface
  MX_FDCAN1_Init();                                                                                                     // Initialize CAN channel 1 and its Standard/Extended global acceptance filter
  MX_FDCAN2_Init();                                                                                                     // Initialize CAN channel 2 and its Standard/Extended global acceptance filter

  /* USER CODE BEGIN 2 */
  bms_setup();                                                                                                          // Configure the ADBMS6830B daisy chain and the original BMS services

  if (can_setup() != 0)                                                                                                 // Start both CAN controllers after CubeMX has configured their global filters
  {
    Error_Handler();                                                                                                    // Stop execution if either CAN peripheral cannot be started
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  for (;;)
  {
    can_receive_all();                                                                                                  // Drain pending Standard and Extended frames before the next BMS acquisition
    bms_loop();                                                                                                         // Read all ADBMS6830B cell-voltage and AUX/temperature channels
    can_receive_all();                                                                                                  // Drain frames accumulated while the SPI acquisition was running
    AMS_Send_DBC();                                                                                                     // Publish valid AMS measurements using IDs 0x501B and 0x701B

    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)                                                                                           // Configures the MCU clock tree used by the original AMS project
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};                                                                           // Oscillator and PLL configuration structure
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};                                                                           // AHB/APB/system clock configuration structure

  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);                                                        // Select regulator voltage scaling used by the existing project

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;                                                            // Use the internal high-speed oscillator
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                                                                              // Enable HSI
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;                                                   // Use the factory/default HSI calibration value
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;                                                                          // Enable the PLL
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;                                                                  // Use HSI as the PLL source
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;                                                                           // Preserve the original PLL input divider
  RCC_OscInitStruct.PLL.PLLN = 16;                                                                                      // Preserve the original PLL multiplier
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;                                                                           // Preserve the original PLL P divider
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;                                                                           // Preserve the original PLL Q divider
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;                                                                           // Preserve the original PLL R divider

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)                                                                  // Apply oscillator/PLL configuration
  {
    Error_Handler();                                                                                                    // Stop on an oscillator configuration failure
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;                                              // Configure the CPU and both APB buses
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;                                                             // Select PLL output as SYSCLK
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;                                                                    // Run AHB at SYSCLK
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;                                                                     // Run APB1 at HCLK
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;                                                                     // Run APB2 at HCLK

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)                                               // Apply bus clocks and Flash latency
  {
    Error_Handler();                                                                                                    // Stop on a clock-tree configuration failure
  }
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)                                                                                                // Fatal-error handler retained from the original AMS project
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();                                                                                                      // Disable interrupts to keep the MCU in a deterministic fault state

  static int first_error = 1;                                                                                           // Prevent recursive TRACE output if the communication layer also fails
  if (first_error != 0)                                                                                                 // Print the fatal error message only once
  {
    first_error = 0;                                                                                                    // Mark the error message as already printed
    TRACE("\n\n\n---FATAL ERROR---\n\n\n");                                                                             // Send a diagnostic marker through the configured TRACE interface
  }

  for (;;)                                                                                                              // Remain in the fault state until the MCU is reset
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number.
 * @param  file Pointer to the source file name.
 * @param  line Assertion line number.
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)                                                                        // Full-assert handler used when USE_FULL_ASSERT is enabled
{
  /* USER CODE BEGIN 6 */
  printf("\n\n\n---FATAL ERROR---\n\n\n### Assertion failed on %s:%lu\n", (char *)file, line);                          // Print the source location that triggered the assertion
  Error_Handler();                                                                                                      // Enter the common fatal-error state
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
