/* USER CODE BEGIN Header */
/**
  *****************************************************************************
  * @file           : fdcan.c
  * @author         : Guillermo Polo
  * @author         : David Hernández Rentero
  * @project        : main
  * @date           : 2026-08-12
  * @editor         : Visual Studio Code
  *
  * @brief          : Program for the FDCAN config
  *****************************************************************************
  * @file    fdcan.c
  * @brief   This file provides code for the configuration
  *          of the FDCAN instances.
  *          The prgtoam is created by Guillermo and modified by David to adapt 
  *          the configuration to the IDs used in the CAN network
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

#include "fdcan.h"                                                                                                      // Incluye la configuración y las definiciones públicas del periférico FDCAN1.
#include "estado.h"

/* USER CODE BEGIN 0 */
#ifndef CAN_TX_TIMEOUT_MS
#define CAN_TX_TIMEOUT_MS 100U                                                                                          // Limita el tiempo máximo de espera cuando la FIFO de transmisión CAN está llena.
#endif
/* USER CODE END 0 */

FDCAN_HandleTypeDef hfdcan1;                                                                                            // Handle HAL que concentra la configuración y el estado de la instancia FDCAN1.

/* FDCAN1 init function */
void MX_FDCAN1_Init(void)                                                                                               // Configura los parámetros principales de funcionamiento del periférico FDCAN1.
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */



  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;                                                                                            // Selecciona la instancia hardware FDCAN1 del STM32.
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;                                                                         // Mantiene el reloj del kernel FDCAN sin división adicional.
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;                                                                       // Selecciona trama CAN clásica en lugar de CAN FD.
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;                                                                                // Configura FDCAN1en modo normal paa recibir mensajes de todos los nodos
  //hfdcan1.Init.Mode = FDCAN_MODE_INTERNAL_LOOPBACK;                                                                   // Configura FDCAN1 en loopback interno para realizar las pruebas presentes en esta versión.
  hfdcan1.Init.AutoRetransmission = DISABLE;                                                                            // Deshabilita la retransmisión automática de una trama cuando se produce un error.
  hfdcan1.Init.TransmitPause = DISABLE;                                                                                 // Deshabilita la pausa adicional entre transmisiones.
  hfdcan1.Init.ProtocolException = DISABLE;                                                                             // Deshabilita el tratamiento de excepciones específico del protocolo CAN FD.
  hfdcan1.Init.NominalPrescaler = 17;                                                                                   // Define el prescaler empleado en el bit timing nominal.
  hfdcan1.Init.NominalSyncJumpWidth = 1;                                                                                // Configura el Synchronization Jump Width nominal a un time quantum.
  hfdcan1.Init.NominalTimeSeg1 = 12;                                                                                     // Define la duración del primer segmento temporal nominal.
  hfdcan1.Init.NominalTimeSeg2 = 7;                                                                                     // Define la duración del segundo segmento temporal nominal.
  hfdcan1.Init.DataPrescaler = 17;                                                                                       // Parámetro de temporización del campo de datos, no utilizado en CAN clásico.
  hfdcan1.Init.DataSyncJumpWidth = 1;                                                                                   // Configura el SJW del campo de datos.
  hfdcan1.Init.DataTimeSeg1 = 12;                                                                                        // Configura el primer segmento temporal del campo de datos.
  hfdcan1.Init.DataTimeSeg2 = 7;                                                                                        // Configura el segundo segmento temporal del campo de datos.
  hfdcan1.Init.StdFiltersNbr = 0;                                                                                       // No reserva filtros específicos para identificadores estándar.
  hfdcan1.Init.ExtFiltersNbr = 1;                                                                                       // No reserva filtros específicos para identificadores extendidos en la configuración generada.
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;                                                               // Configura el buffer de transmisión para operar como FIFO.
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)                                                                               // Inicializa FDCAN1 mediante la librería HAL y comprueba el resultado.
  {
    Error_Handler();                                                                                                    // Deriva al manejador de error si la inicialización del periférico falla.
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* fdcanHandle)                                                                // Inicializa recursos de bajo nivel necesarios por la instancia FDCAN indicada.
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};                                                                               // Estructura utilizada para configurar los pines asociados al periférico.
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};                                                                         // Estructura utilizada para seleccionar la fuente de reloj de FDCAN.
  if(fdcanHandle->Instance==FDCAN1)                                                                                     // Limita esta inicialización de bajo nivel exclusivamente a FDCAN1.
  {
  /* USER CODE BEGIN FDCAN1_MspInit 0 */

  /* USER CODE END FDCAN1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;                                                           // Selecciona FDCAN como periférico cuyo reloj será configurado.
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;                                                       // Utiliza PCLK1 como fuente de reloj del periférico FDCAN.
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)                                                            // Aplica la selección de reloj periférico y verifica que la operación sea correcta.
    {
      Error_Handler();                                                                                                  // Ejecuta el manejador global de error si no puede configurarse el reloj.
    }

    /* FDCAN1 clock enable */
    __HAL_RCC_FDCAN_CLK_ENABLE();                                                                                       // Habilita el reloj de la unidad FDCAN1.

    __HAL_RCC_GPIOA_CLK_ENABLE();                                                                                       // Habilita el reloj del puerto GPIOA utilizado por RX y TX.
    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;                                                                      // Selecciona PA11 y PA12 para las señales RX y TX de FDCAN1.
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;                                                                             // Configura ambos pines en función alternativa push-pull.
    GPIO_InitStruct.Pull = GPIO_NOPULL;                                                                                 // Mantiene RX y TX sin resistencias internas de polarización.
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;                                                                        // Selecciona una velocidad GPIO baja para las señales configuradas.
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;                                                                        // Asigna la función alternativa AF9 correspondiente a FDCAN1.
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                                                                             // Aplica la configuración de función alternativa a PA11 y PA12.

    /* FDCAN1 interrupt Init */
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 1, 0);                                                                        // Asigna prioridad a la primera línea de interrupción de FDCAN1.
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);                                                                                // Habilita la primera interrupción de FDCAN1 en el NVIC.
    HAL_NVIC_SetPriority(FDCAN1_IT1_IRQn, 1, 0);                                                                        // Asigna prioridad a la segunda línea de interrupción de FDCAN1.
    HAL_NVIC_EnableIRQ(FDCAN1_IT1_IRQn);                                                                                // Habilita la segunda interrupción de FDCAN1 en el NVIC.
  /* USER CODE BEGIN FDCAN1_MspInit 1 */

  /* USER CODE END FDCAN1_MspInit 1 */
  }
}

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* fdcanHandle)                                                              // Libera los recursos de bajo nivel asociados a una instancia FDCAN.
{

  if(fdcanHandle->Instance==FDCAN1)                                                                                     // Ejecuta la desinicialización únicamente cuando el handle corresponde a FDCAN1.
  {
  /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

  /* USER CODE END FDCAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_FDCAN_CLK_DISABLE();                                                                                      // Deshabilita el reloj del periférico FDCAN1.

    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);                                                                    // Devuelve PA11 y PA12 a su estado de desinicialización.

    /* FDCAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);                                                                               // Deshabilita la primera línea de interrupción de FDCAN1.
    HAL_NVIC_DisableIRQ(FDCAN1_IT1_IRQn);                                                                               // Deshabilita la segunda línea de interrupción de FDCAN1.
  /* USER CODE BEGIN FDCAN1_MspDeInit 1 */

  /* USER CODE END FDCAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
FDCAN_RxHeaderTypeDef RxHeader;                                                                                         // Cabecera donde HAL almacena metadatos de la última trama CAN recibida.
uint8_t RxData[8];                                                                                                      // Buffer de ocho bytes utilizado para almacenar los datos de recepción CAN.
volatile uint8_t can_ok = 0;                                                                                            // Flag compartida que indica la recepción de la orden de activación del Ready To Drive.

void FDCAN_ConfigFilter(void)                                                                                           // Configura el filtrado empleado para dirigir las tramas recibidas hacia RX FIFO0.
{
  FDCAN_FilterTypeDef sFilterConfig;                                                                                    // Estructura local utilizada para definir el filtro FDCAN.
  sFilterConfig.FilterIndex = 0;                                                                                        // Selecciona el primer índice disponible de filtro.
  sFilterConfig.FilterType = FDCAN_FILTER_MASK;                                                                         // Selecciona un filtro de tipo máscara.
  sFilterConfig.IdType = FDCAN_EXTENDED_ID;                                                                             // Indica que el filtro trabaja con identificadores CAN extendidos de 29 bits.
  sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;                                                                 // Dirige las tramas aceptadas hacia la FIFO de recepción 0.
  sFilterConfig.FilterID1 = 0x00000000;                                                                                 // Fija a cero el identificador de referencia del filtro.
  sFilterConfig.FilterID2 = 0x00000000;                                                                                 // Utiliza máscara nula, por lo que no se discrimina ningún bit del identificador.
  HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig);                                                                     // Aplica al periférico la configuración de filtro definida.
  HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_ACCEPT_IN_RX_FIFO0,                                                      // Configura el comportamiento global para las tramas no asociadas a un filtro específico.
                                FDCAN_ACCEPT_IN_RX_FIFO0,
                                FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);
}

void FDCAN_Start(void)                                                                                                  // Arranca el periférico FDCAN y habilita la recepción mediante interrupciones.
{
  HAL_FDCAN_Start(&hfdcan1);                                                                                            // Pone en funcionamiento FDCAN1 una vez finalizada su configuración.
  HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);                                           // Activa la notificación cuando llega un nuevo mensaje a RX FIFO0.
}

void FDCAN_SendMessage(uint32_t id, uint8_t *data, uint32_t len)                                                        // Construye y encola una trama CAN con el identificador, datos y longitud indicados.
{
    FDCAN_TxHeaderTypeDef TxHeader = {0};                                                                               // Crea e inicializa la cabecera utilizada para configurar la trama CAN.

    TxHeader.Identifier = id;                                                                                           // Asigna el identificador CAN recibido como argumento.
    TxHeader.IdType = FDCAN_EXTENDED_ID;                                                                                // Configura la transmisión mediante identificadores extendidos de 29 bits.
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;                                                                            // Selecciona una trama CAN de datos.
    TxHeader.DataLength = len;                                                                                          // Establece la longitud de la trama mediante el DLC recibido.
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;                                                                    // Indica que el nodo se encuentra en estado de error activo.
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;                                                                             // Deshabilita Bit Rate Switching al utilizar CAN clásico.
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;                                                                              // Selecciona explícitamente el formato CAN clásico.
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;                                                                   // Deshabilita el almacenamiento de eventos de transmisión.
    TxHeader.MessageMarker = 0;                                                                                         // No utiliza marcador adicional para identificar la trama.

    uint32_t tx_wait_start_tick = HAL_GetTick();                                                                        // Guarda el instante en el que comienza la espera de espacio en la FIFO.

    while (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) == 0U)                                                               // Espera mientras no exista espacio disponible en la FIFO de transmisión.
    {
        if ((uint32_t)(HAL_GetTick() - tx_wait_start_tick) >= CAN_TX_TIMEOUT_MS)                                        // Comprueba que la espera no supere el tiempo máximo establecido.
        {
            Error_Handler();                                                                                            // Ejecuta el manejador de error si la FIFO permanece bloqueada.
        }
    }

    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, data) != HAL_OK)                                             // Introduce la trama preparada en la FIFO de transmisión y comprueba el resultado.
    {
        Error_Handler();                                                                                                // Ejecuta el manejador de error si la transmisión no puede ser encolada.
    }
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)                                        // Callback ejecutado por HAL cuando se genera una interrupción asociada a RX FIFO0.
{
  if(RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE)                                                                        // Comprueba que la causa de la interrupción sea la llegada de un nuevo mensaje.
  {
    HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData);                                                  // Extrae de RX FIFO0 la cabecera y los ocho bytes de la trama recibida.
    if(RxHeader.Identifier == 0x0000301B)                                                                               // Detecta el identificador utilizado como orden de activación de la secuencia R2D.
    {
      if ((RxData[4] & 0x10U) != 0U)
      {
          HAL_GPIO_WritePin(LED_AVISO1_GPIO_Port, LED_AVISO1_Pin, GPIO_PIN_SET);                                        // Activa el LED de aviso al reconocer la orden de activación.
          can_ok = 1;                                                                                                   // Levanta la flag consumida posteriormente por el estado ESPERA.
      }                                                                                                       
    }
    // Mensaje del inversor — voltaje en bytes 6-7, escala 1 (valor directo en V)
    if(RxHeader.Identifier == 0x0000221B)                                                                               // Comprueba si la trama corresponde al mensaje de datos generales del inversor esperado.
    {
      uint16_t voltaje_v = (RxData[6] << 8) | RxData[7];                                                                // Reconstruye el valor de 16 bits a partir de los bytes 6 y 7 en orden MSB-LSB.
      voltaje_inversor_mV = voltaje_v;                                                                                  // Actualiza la variable global utilizada por la lógica de precarga.
    }
    else if(RxHeader.Identifier == 0x0000701B)                                                                          // Comprueba si la trama corresponde al mensaje utilizado para la tensión de batería.
    {
      uint16_t v_bat_raw = (uint16_t)((RxData[0] << 8) | RxData[1]);                                                    // Reconstruye el valor bruto de batería a partir de los dos primeros bytes recibidos.
      if(v_bat_raw != 0xFFFF) // Si el dato es válido
      {
        // Aplicando Fórmula: (Raw * 0.05) - 1606.4
        // Del AMS creo que se puede obtener directemante ne lugar de realizar el ajuste que hicimios de la fuente
        float v_bat_f = (v_bat_raw * 0.05f) - 1606.4f;                                                                  // Aplica al valor bruto la ecuación de conversión implementada en el proyecto.
        voltaje_bateria_mV = v_bat_f;                                                                                   // Actualiza la variable global de tensión de batería con el resultado convertido.
      }
    }
  }
}


void FDCAN_SendR2DStatus(void)                                                                                          // Transmite periódicamente por CAN el estado completo de la secuencia Ready To Drive.
{
    static uint32_t lastTick = 0U;                                                                                      // Almacena el instante correspondiente a la última transmisión del mensaje R2D.
    uint32_t now = HAL_GetTick();                                                                                       // Obtiene el tiempo actual del sistema desde el arranque del microcontrolador.

    if ((now - lastTick) >= CAN_R2D_STATUS_PERIOD_MS)                                                                  // Comprueba si se ha cumplido el periodo establecido para transmitir el estado R2D.
    {
        uint8_t TxData[8] = {0};                                                                                        // Inicializa a cero los ocho bytes que componen la trama CAN.

        uint16_t r2d_status = (uint16_t)estado;                                                                         // Obtiene el estado general actual de la máquina de estados R2D.
        uint16_t r2d_internal_state = (uint16_t)fase;                                                                   // Obtiene la subfase actual de la secuencia interna de precarga.
        uint16_t r2d_relay_status = 0U;                                                                                 // Inicializa la variable que agrupa mediante bits el estado de los diferentes relés.
        uint16_t r2d_error_status = codigo_error;                                                                       // Obtiene el código de error actualmente registrado por el sistema.

        /* R2D_Status - Bytes 0 y 1 */
        TxData[0] = (uint8_t)(r2d_status >> 8);                                                                         // Introduce en el byte 0 la parte más significativa del estado general R2D.
        TxData[1] = (uint8_t)(r2d_status & 0xFFU);                                                                      // Introduce en el byte 1 la parte menos significativa del estado general R2D.

        /* R2D_InternalState - Bytes 2 y 3 */
        TxData[2] = (uint8_t)(r2d_internal_state >> 8);                                                                 // Introduce en el byte 2 la parte más significativa del estado interno de la secuencia.
        TxData[3] = (uint8_t)(r2d_internal_state & 0xFFU);                                                              // Introduce en el byte 3 la parte menos significativa del estado interno de la secuencia.

        /* R2D_RelayStatus - Bytes 4 y 5 */
        if (dbg_air_high)                                                                                               // Comprueba mediante el feedback si el contactor AIR+ se encuentra cerrado.
            r2d_relay_status |= R2D_AIR_POS_MASK;                                                                       // Activa el bit 0 correspondiente al estado del contactor AIR+.

        if (dbg_air_low)                                                                                                // Comprueba mediante el feedback si el contactor AIR- se encuentra cerrado.
            r2d_relay_status |= R2D_AIR_NEG_MASK;                                                                       // Activa el bit 1 correspondiente al estado del contactor AIR-.

        if (dbg_precharge)                                                                                              // Comprueba mediante el feedback si el relé de precarga se encuentra cerrado.
            r2d_relay_status |= R2D_PRECHARGE_MASK;                                                                     // Activa el bit 2 correspondiente al estado del relé de precarga.

        if (HAL_GPIO_ReadPin(OUTPUT_DISCHARGE_GPIO_Port, OUTPUT_DISCHARGE_Pin) == GPIO_PIN_SET)                          // Comprueba el estado comandado del relé de descarga al no disponer de feedback físico.
            r2d_relay_status |= R2D_DISCHARGE_MASK;                                                                     // Activa el bit 3 correspondiente al estado comandado del relé de descarga.

        TxData[4] = (uint8_t)(r2d_relay_status & 0xFFU);                                                                // Introduce en el byte 4 los bits correspondientes al estado de los relés.
        TxData[5] = 0x00U;                                                                                              // Mantiene reservado el byte 5 correspondiente a R2D_RelayStatus.

        /* R2D_ErrorStatus - Bytes 6 y 7 */
        TxData[6] = (uint8_t)(r2d_error_status >> 8);                                                                   // Introduce en el byte 6 la parte más significativa del código de error.
        TxData[7] = (uint8_t)(r2d_error_status & 0xFFU);                                                                // Introduce en el byte 7 la parte menos significativa del código de error.

        FDCAN_SendMessage(CAN_ID_R2D, TxData, FDCAN_DLC_BYTES_8);                                                       // Envía los ocho bytes del estado R2D utilizando el identificador definido en el DBC.

        lastTick = now;                                                                                                 // Actualiza la referencia temporal utilizada para controlar la siguiente transmisión.
    }
}


void FDCAN_SendRTDOk(void)                                                                                              // Envía periódicamente la señal CAN que representa el estado RTD correcto.
{
  static uint32_t lastTick = 0;                                                                                         // Conserva el instante de la última transmisión entre llamadas sucesivas.
  uint32_t now = HAL_GetTick();                                                                                         // Obtiene el tiempo actual del sistema en milisegundos.

  if ((now - lastTick) >= CAN_RTD_OK_PERIOD_MS)                                                                         // Comprueba que haya transcurrido el periodo de transmisión configurado.
  {
    uint8_t TxData[8] = {0};                                                                                            // Inicializa a cero el buffer local de ocho bytes.

    TxData[0] = CAN_RTD_OK_VALUE;                                                                                       // Carga en el primer byte el valor definido para indicar RTD correcto.

    FDCAN_SendMessage(                                                                                                  // Solicita el envío de la trama mediante la función genérica de transmisión FDCAN.
        CAN_RTD_OK_ID,
        TxData,
        FDCAN_DLC_BYTES_8
    );

    lastTick = now;                                                                                                     // Actualiza el instante de la última transmisión para mantener la periodicidad.
  }
}
/* USER CODE END 1 */
