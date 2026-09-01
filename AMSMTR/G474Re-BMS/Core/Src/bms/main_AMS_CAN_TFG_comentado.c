/**
 ******************************************************************************
 * @file           : main_AMS_TFG_reducido.c
 * @brief          : Versión reducida del programa principal del AMS.
 * @brief          : Fragmento preparado para su inclusión en la memoria del TFG.
 *
 * @note           : Se han omitido las rutinas generadas por STM32CubeMX,
 *                   la redirección de printf, la configuración completa del
 *                   reloj y la gestión detallada de errores, ya que no forman
 *                   parte de la lógica funcional analizada en este apartado.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"                                                                                                       // Definiciones generales del proyecto
#include "fdcan.h"                                                                                                      // Interfaces FDCAN1 y FDCAN2
#include "gpio.h"                                                                                                       // Inicialización de los GPIO
#include "spi.h"                                                                                                        // SPI utilizado por la cadena ADBMS6830B
#include "usart.h"                                                                                                      // Interfaz serie de diagnóstico

#include "bms/bms.h"                                                                                                    // Funciones principales de adquisición del BMS
#include "bms/can.h"                                                                                                    // Funciones de recepción y transmisión CAN
#include "bms/config.h"                                                                                                 // Configuración de la cadena de ADBMS6830B

#include <stdbool.h>                                                                                                    // Tipo booleano
#include <stdint.h>                                                                                                     // Tipos enteros de tamaño fijo


/* Dimensionado de las variables de medida ----------------------------------*/

enum
{
  AMS_CELLS_PER_ADBMS = 16U,                                                                                            // Número de celdas supervisadas por cada ADBMS6830B
  AMS_NUM_CELLS = NUM_DAISY_ADBMS * AMS_CELLS_PER_ADBMS,                                                               // Número total de celdas del acumulador

  AMS_NUM_TEMPERATURES =                                                                                                // Número total de canales destinados a temperatura
      ((NUM_DAISY_ADBMS - 1U) * NUM_MUX_CHANNELS * NUM_MUX_EXCEPT_LAST) +
      (NUM_MUX_CHANNELS * NUM_MUX_LAST)
};


/* Identificadores CAN definidos en el DBC ----------------------------------*/

#define CAN_ID_AMS_TEMPERATURES          0x0000501BU                                                                    // Temperature_max + Temperature_min
#define CAN_ID_AMS_BATTERY_MEASUREMENTS  0x0000701BU                                                                    // Battery_Current + Battery_Voltage
#define AMS_DBC_TX_PERIOD_MS             100U                                                                           // Periodo mínimo entre transmisiones del AMS

#define AMS_DBC_FDCAN_HANDLE             hfdcan1                                                                        // Interfaz CAN utilizada para transmitir a la red del vehículo


/* Variables principales supervisadas por el AMS ----------------------------*/

/* Tensiones de las celdas. */
volatile float AMS_cell_voltage[AMS_NUM_CELLS] = {0.0f};                                                               // Tensiones individuales de las celdas
volatile float AMS_battery_voltage = 0.0f;                                                                              // Tensión total del acumulador
volatile float AMS_voltage_min = 0.0f;                                                                                  // Tensión mínima de celda
volatile float AMS_voltage_max = 0.0f;                                                                                  // Tensión máxima de celda
volatile bool  AMS_voltage_valid = false;                                                                               // Validez de la última adquisición de tensiones

/* Temperaturas del acumulador. */
volatile float AMS_temperature[AMS_NUM_TEMPERATURES] = {0.0f};                                                          // Temperaturas individuales medidas
volatile float AMS_temperature_min = 0.0f;                                                                             // Temperatura mínima del acumulador
volatile float AMS_temperature_max = 0.0f;                                                                             // Temperatura máxima del acumulador
volatile bool  AMS_temperature_valid = false;                                                                           // Validez de la última adquisición de temperaturas

/* Corriente del acumulador recibida mediante CAN. */
volatile float AMS_battery_current = 0.0f;                                                                              // Corriente instantánea del acumulador
volatile bool  AMS_current_valid = false;                                                                               // Validez de la última medida de corriente

static uint32_t AMS_last_dbc_tx_tick = 0U;                                                                              // Marca temporal para limitar la frecuencia de transmisión


/* Prototipos de las funciones empleadas ------------------------------------*/

void SystemClock_Config(void);                                                                                          // Configuración del reloj generada mediante CubeMX
error can_setup(void);                                                                                                  // Inicialización de las interfaces CAN
void can_receive_all(void);                                                                                            // Procesamiento de las tramas recibidas

HAL_StatusTypeDef can_send_frame(FDCAN_HandleTypeDef *hfdcan,
                                 uint32_t id,
                                 uint32_t id_type,
                                 const uint8_t *data,
                                 uint32_t data_length);                                                                 // Transmisión genérica Standard/Extended

static void     AMS_Send_DBC(void);                                                                                    // Transmisión de las variables principales del AMS
static void     Write_BE_U32(uint8_t *data, uint32_t value);                                                           // Conversión a orden Motorola/Big-Endian
static uint32_t DBC_Encode_Temperature(float temperature_c);                                                           // Codificación de temperaturas
static uint32_t DBC_Encode_Current(float current_a);                                                                   // Codificación de corriente
static uint32_t DBC_Encode_Voltage(float voltage_v);                                                                   // Codificación de tensión


/* Conversión de variables físicas al formato definido por el DBC -----------*/

static void Write_BE_U32(uint8_t *data, uint32_t value)                                                                 // Escribe un valor de 32 bits en formato Big-Endian
{
  data[0] = (uint8_t)(value >> 24U);                                                                                    // Bits 31..24
  data[1] = (uint8_t)(value >> 16U);                                                                                    // Bits 23..16
  data[2] = (uint8_t)(value >> 8U);                                                                                     // Bits 15..8
  data[3] = (uint8_t)value;                                                                                             // Bits 7..0
}

static uint32_t DBC_Encode_Temperature(float temperature_c)                                                             // Temperature = RAW * 0,1 - 10
{
  return (uint32_t)(((temperature_c + 10.0f) * 10.0f) + 0.5f);                                                         // Conversión inversa al valor RAW del DBC
}

static uint32_t DBC_Encode_Current(float current_a)                                                                     // Battery_Current = RAW * 0,1
{
  return (uint32_t)((current_a * 10.0f) + 0.5f);                                                                       // Conversión inversa al valor RAW del DBC
}

static uint32_t DBC_Encode_Voltage(float voltage_v)                                                                     // Battery_Voltage = RAW * 0,01
{
  return (uint32_t)((voltage_v * 100.0f) + 0.5f);                                                                      // Conversión inversa al valor RAW del DBC
}


/* Transmisión de las variables principales del AMS -------------------------*/

static void AMS_Send_DBC(void)
{
  const uint32_t now = HAL_GetTick();                                                                                   // Obtiene la marca temporal actual

  if ((uint32_t)(now - AMS_last_dbc_tx_tick) < AMS_DBC_TX_PERIOD_MS)                                                    // Comprueba la periodicidad de transmisión
  {
    return;                                                                                                             // No transmite si todavía no ha transcurrido el periodo establecido
  }

  AMS_last_dbc_tx_tick = now;                                                                                           // Actualiza la marca temporal de transmisión

  /* Mensaje 0x501B: temperatura máxima y temperatura mínima. */
  if (AMS_temperature_valid)                                                                                            // Solo transmite cuando las temperaturas son válidas
  {
    uint8_t tx_data[8] = {0U};                                                                                          // Buffer de ocho bytes correspondiente al mensaje

    Write_BE_U32(&tx_data[0], DBC_Encode_Temperature(AMS_temperature_max));                                             // Bytes 0..3: Temperature_max
    Write_BE_U32(&tx_data[4], DBC_Encode_Temperature(AMS_temperature_min));                                             // Bytes 4..7: Temperature_min

    can_send_frame(&AMS_DBC_FDCAN_HANDLE,
                   CAN_ID_AMS_TEMPERATURES,
                   FDCAN_EXTENDED_ID,
                   tx_data,
                   FDCAN_DLC_BYTES_8);                                                                                  // Transmite el mensaje 0x501B mediante CAN Extended
  }

  /* Mensaje 0x701B: corriente y tensión total del acumulador. */
  if (AMS_voltage_valid && AMS_current_valid)                                                                           // Solo transmite si ambas magnitudes son válidas
  {
    uint8_t tx_data[8] = {0U};                                                                                          // Buffer de ocho bytes correspondiente al mensaje

    Write_BE_U32(&tx_data[0], DBC_Encode_Current(AMS_battery_current));                                                 // Bytes 0..3: Battery_Current
    Write_BE_U32(&tx_data[4], DBC_Encode_Voltage(AMS_battery_voltage));                                                 // Bytes 4..7: Battery_Voltage

    can_send_frame(&AMS_DBC_FDCAN_HANDLE,
                   CAN_ID_AMS_BATTERY_MEASUREMENTS,
                   FDCAN_EXTENDED_ID,
                   tx_data,
                   FDCAN_DLC_BYTES_8);                                                                                  // Transmite el mensaje 0x701B mediante CAN Extended
  }
}


/* Programa principal -------------------------------------------------------*/

int main(void)
{
  HAL_Init();                                                                                                           // Inicializa la HAL y los periféricos básicos del microcontrolador
  SystemClock_Config();                                                                                                 // Configura el reloj del sistema

  MX_GPIO_Init();                                                                                                       // Inicializa los GPIO de la placa
  MX_SPI1_Init();                                                                                                       // Inicializa la comunicación con el ADBMS6830B
  MX_LPUART1_UART_Init();                                                                                               // Inicializa la interfaz de diagnóstico
  MX_FDCAN1_Init();                                                                                                     // Inicializa la interfaz CAN de la red del vehículo
  MX_FDCAN2_Init();                                                                                                     // Inicializa la interfaz CAN utilizada por el IMD

  bms_setup();                                                                                                          // Configura la cadena de monitorización del acumulador

  if (can_setup() != 0)                                                                                                 // Arranca ambas interfaces CAN
  {
    Error_Handler();                                                                                                    // Entra en estado seguro si falla la inicialización
  }

  for (;;)
  {
    can_receive_all();                                                                                                  // Procesa las tramas CAN recibidas
    bms_loop();                                                                                                         // Adquiere tensiones y temperaturas del acumulador
    can_receive_all();                                                                                                  // Procesa las tramas acumuladas durante la adquisición
    AMS_Send_DBC();                                                                                                     // Transmite las variables principales del AMS
  }
}


/* Las funciones SystemClock_Config(), Error_Handler(), redirección de printf
 * y demás rutinas auxiliares generadas por STM32CubeMX se omiten en la memoria
 * por no formar parte de la lógica funcional presentada en este apartado. */
