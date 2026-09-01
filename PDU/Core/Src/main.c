/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"                                                                                                       // Incluye las definiciones generales de la aplicación y del microcontrolador.

#include <stdio.h>                                                                                                      // Habilita las funciones de entrada/salida estándar utilizadas para el diagnóstico por terminal.

#include "estado.h"                                                                                                     // Incluye la interfaz de la máquina de estados del Ready To Drive.
#include "fdcan.h"                                                                                                      // Incluye las funciones de inicialización, recepción y transmisión FDCAN.
#include "usart.h"                                                                                                      // Incluye la configuración de la interfaz UART utilizada por printf().
#include "gpio.h"                                                                                                       // Incluye la configuración de los pines GPIO del sistema.

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE BEGIN PD */

/* Identificador del mensaje R2D */
#define CAN_ID_R2D                 0x0000301BU                                                                          // Id de CAN para el R2D

/* Periodo de transmisión del estado R2D */
#define CAN_R2D_STATUS_PERIOD_MS   100U                                                                                 // Timeout de CAN

/* Máscaras de R2D_RelayStatus - Byte 4 */
#define R2D_AIR_POS_MASK           (1U << 0)                                                                            // Bit 0: Estado AIR+
#define R2D_AIR_NEG_MASK           (1U << 1)                                                                            // Bit 1: Estado AIR-
#define R2D_PRECHARGE_MASK         (1U << 2)                                                                            // Bit 2: Estado del relé de precarga
#define R2D_DISCHARGE_MASK         (1U << 3)                                                                            // Bit 3: Estado del relé de descarga
#define R2D_ACTIVATION_MASK        (1U << 4)                                                                            // Bit 4: Petición de activación R2D desde la VCU

/* USER CODE END PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint8_t lv_check_pending = 0;                                                                                  // Flag modificada desde la interrupción EXTI para solicitar una nueva comprobación de seguridad LV.
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);                                                                                          // Prototipo de la función encargada de configurar el reloj del sistema.
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)                                                                                                // Redirige la salida carácter a carácter utilizada por printf hacia la UART2.
{
  HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);                                                          // Transmite por UART2 el carácter recibido y espera hasta completar la operación.
  return ch;                                                                                                            // Devuelve el carácter transmitido para mantener la interfaz esperada por la librería estándar.
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)                                                                                                          // Punto de entrada principal del firmware.
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();                                                                                                           // Inicializa HAL, SysTick y los recursos básicos del microcontrolador.

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();                                                                                                 // Configura la fuente y distribución del reloj del sistema.

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();                                                                                                       // Inicializa las entradas y salidas digitales definidas para la PDU/R2D.
  MX_FDCAN1_Init();                                                                                                     // Inicializa el periférico FDCAN1 con los parámetros configurados.
  MX_USART2_UART_Init();                                                                                                // Inicializa USART2 para disponer de salida de diagnóstico por terminal.
  /* USER CODE BEGIN 2 */
  uint16_t sim_v_inv = 0;    // Voltaje inversor inicial (0V)                                                           // Variable empleada por el bloque de simulación del voltaje del inversor.
  uint16_t sim_v_bat_raw = 32128; // Voltaje batería inicial (corresponde a 0V según offset)                            // Valor bruto inicial empleado por el bloque de simulación de la batería.
  //Implementacion canbus
  /*FDCAN_ConfigFilter();
  FDCAN_Start();
  printf("\r\n=== Ready To Drive - CAN Test ===\r\n");
  uint8_t TxData[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  FDCAN_SendMessage(0x18FF0001, TxData, FDCAN_DLC_BYTES_8);
  printf("Mensaje CAN enviado ID=0x18FF0001\r\n");*/
  FDCAN_ConfigFilter();                                                                                                 // Configura el filtro de recepción CAN utilizado por la aplicación.
  FDCAN_Start();                                                                                                        // Arranca FDCAN1 y activa la notificación de nuevos mensajes recibidos.
  /*printf("\r\n=== Ready To Drive - CAN Test ===\r\n");                                                                  // Muestra por UART el inicio de la prueba CAN.
  uint8_t TxData[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x86};                                                 // Construye una trama de prueba con el valor situado en los bytes 6 y 7.
  FDCAN_SendMessage(0x00002022, TxData, FDCAN_DLC_BYTES_8);                                                             // Envía la trama de prueba con el identificador empleado para el mensaje del inversor.
  printf("Mensaje CAN enviado \r\n");                                                                                   // Confirma por terminal el envío de la trama de prueba.

  printf("\r\n=== Ready To Drive - CAN Test (BATERIA) ===\r\n");                                                        // Muestra el inicio de la prueba asociada al mensaje de batería.
  // Valor calculado para 400V -> 40128 (0x9CC0)
  // Según la tabla, este valor suele ir en los dos primeros bytes
  uint8_t TxDataBat[8] = {0x9C, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};                                              // Construye la trama de batería utilizada durante la prueba de recepción.
  // Enviamos con el ID del sistema de batería (ejemplo: 0x18FF5E00)
  // Asegúrate de usar el ID correcto que espera tu receptor
  FDCAN_SendMessage(0x18FF5E00, TxDataBat, FDCAN_DLC_BYTES_8);                                                          // Envía la trama de prueba con el identificador definido para la tensión de batería.
  printf("Mensaje CAN Bateria (400V) enviado con ID=0x18FF5E00\r\n");                                                   // Informa por terminal del envío del mensaje de batería simulado.

  uint8_t TxDataActivacion[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};                                       // Construye la trama empleada para simular la orden de activación R2D.
  FDCAN_SendMessage(0x18FF0001, TxDataActivacion, FDCAN_DLC_BYTES_8);                                                   // Envía la orden de activación que será detectada por el callback de recepción.
  printf("[TEST] Mensaje CAN OK enviado (ID=0x18FF0001)\r\n");                                                          // Confirma por terminal el envío de la trama de activación.*/


  estado_inicial_HV();                                                                                                  // Establece la condición inicial de los relés del sistema de alta tensión.
  verificar_estado_LV();                                                                                                // Realiza la primera comprobación del circuito de seguridad de baja tensión.

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)                                                                                                             // Bucle principal que ejecuta de forma continua la lógica de supervisión y estados.
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    /* --- SIMULADOR DE MENSAJES CAN --- */

    // 1. Simular Inversor (ID 0x2022) - Sube de 5 en 5V hasta 400V
    /*uint8_t TxDataInv[8] = {0, 0, 0, 0, 0, 0, (uint8_t)(sim_v_inv >> 8), (uint8_t)(sim_v_inv & 0xFF)};
    FDCAN_SendMessage(0x00002022, TxDataInv, FDCAN_DLC_BYTES_8);

    sim_v_inv += 5;
    if (sim_v_inv > 400) sim_v_inv = 0;

    HAL_Delay(500); // Pausa para no saturar el log

    // 2. Simular Batería (ID 0x18FF5E00) - Sube de 10 en 10 bits
    uint8_t TxDataBat[8] = {(uint8_t)(sim_v_bat_raw >> 8), (uint8_t)(sim_v_bat_raw & 0xFF), 0, 0, 0, 0, 0, 0};
    FDCAN_SendMessage(0x18FF5E00, TxDataBat, FDCAN_DLC_BYTES_8);

    sim_v_bat_raw += 10;
    if (sim_v_bat_raw > 45000) sim_v_bat_raw = 32128; // Reiniciar al llegar a ~640V
    */

    // --- LOG DE RECEPCIÓN ---
    if(can_ok)                                                                                                          // Comprueba si el callback CAN ha detectado la orden de activación.
    {
      printf("\r\n[CAN RX] ID: 0x%08lX | Inv: %lu mV | Bat: %lu mV\r\n",                                                // Muestra por terminal el identificador recibido y los valores de tensión disponibles.
             RxHeader.Identifier, voltaje_inversor_mV, voltaje_bateria_mV);
    /** 
    *  @attention can_ok ya NO se pone a 0 aquí. Solo estado_espera() 
    *  (en estado.c) debe consumirla, para no robarle la flag a la máquina 
    *  de estados.
    * */
    }

    if (lv_check_pending) {                                                                                             // Comprueba si la interrupción GPIO ha solicitado volver a evaluar la seguridad LV.
      lv_check_pending = 0;                                                                                             // Consume la petición pendiente para evitar repetir la comprobación sin un nuevo evento.
      verificar_estado_LV();                                                                                            // Ejecuta la comprobación del circuito de seguridad tras el evento EXTI.
    }
    estado_maquina();                                                                                                   // Ejecuta una iteración de la máquina de estados del sistema R2D.
    FDCAN_SendR2DStatus();                                                                                              // Ejecuta la función para mandar los valores del R2D
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)                                                                                           // Configura el árbol de reloj utilizado por el microcontrolador.
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};                                                                           // Estructura HAL que contiene la configuración de los osciladores y del PLL.
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};                                                                           // Estructura HAL que contiene la distribución del reloj hacia CPU y buses.

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);                                                        // Selecciona la escala de tensión interna del regulador para la frecuencia configurada.

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;                                                            // Selecciona el oscilador interno HSI como fuente base.
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                                                                              // Activa el oscilador interno HSI.
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;                                                   // Mantiene el valor de calibración HSI predeterminado por HAL.
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;                                                                          // Activa el PLL para generar el reloj del sistema.
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;                                                                  // Utiliza HSI como señal de entrada del PLL.
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;                                                                           // Configura el divisor de entrada M del PLL.
  RCC_OscInitStruct.PLL.PLLN = 10;                                                                                      // Configura el multiplicador N del PLL.
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;                                                                           // Configura la salida P del PLL.
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;                                                                           // Configura la salida Q del PLL.
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;                                                                           // Configura la salida R del PLL utilizada por el reloj del sistema.
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)                                                                  // Aplica la configuración del oscilador y comprueba el resultado devuelto por HAL.
  {
    Error_Handler();                                                                                                    // Invoca el manejador de error si falla la configuración del oscilador.
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK                                                 // Selecciona los dominios de reloj que serán configurados.
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;                                                             // Selecciona la salida del PLL como reloj principal del sistema.
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;                                                                    // Mantiene HCLK sin división respecto al reloj del sistema.
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;                                                                     // Mantiene PCLK1 sin división respecto a HCLK.
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;                                                                     // Mantiene PCLK2 sin división respecto a HCLK.

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)                                               // Aplica la configuración de buses y la latencia de Flash indicada.
  {
    Error_Handler();                                                                                                    // Invoca el manejador de error si la configuración de reloj no puede aplicarse.
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){                                                                         // Callback ejecutado por HAL al producirse una interrupción externa GPIO.
  lv_check_pending = 1;                                                                                                 // Marca como pendiente una nueva comprobación del circuito de seguridad LV.
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)                                                                                                // Manejador global utilizado cuando una operación HAL crítica devuelve error.
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();                                                                                                      // Deshabilita las interrupciones para dejar el sistema detenido en una condición controlada.
  while (1)                                                                                                             // Mantiene la ejecución bloqueada tras detectarse un error crítico.
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)                                                                        // Función de diagnóstico utilizada por HAL cuando están habilitadas las aserciones completas.
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
