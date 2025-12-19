/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  * @brief          : Implements Relative Current Control (0-100%)
  * @brief          : Sets Safety Limits: Max AC = 15A, Max Brake = 0A
  * @brief          : FIXED: Node ID reverted to 0x1B
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "fdcan.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h> // For memset
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
#define REL_CURRENT_MIN 0
#define REL_CURRENT_MAX 100 * 10

#define LIMIT_MAX_AC_CURRENT 15 * 10
#define LIMIT_MAX_BRAKE_CURRENT 0 * 10

#define NODE_ID 0x1B

#define PACKET_ID_SET_REL_CURRENT      0x05
#define PACKET_ID_SET_MAX_AC           0x08
#define PACKET_ID_SET_MAX_BRAKE        0x09
#define PACKET_ID_DRIVE_ENABLE         0x0C
/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
extern ADC_HandleTypeDef hadc1;
extern FDCAN_HandleTypeDef hfdcan1;

FDCAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[8];

uint16_t adc_value = 0;
uint16_t target_rel_current = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Send_CAN_Message(uint32_t id, uint8_t* data, uint32_t len);
long map(long x, long in_min, long in_max, long out_min, long out_max);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    if (in_max == in_min) return out_min;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
/* USER CODE END 0 */

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_FDCAN1_Init();
    MX_ADC1_Init();

    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) Error_Handler();
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK) Error_Handler();

    // Configure global CAN header
    TxHeader.IdType = FDCAN_EXTENDED_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    // --- 1. Send safety limits ---
    memset(TxData, 0, 8);

    // Max AC
    TxData[0] = (LIMIT_MAX_AC_CURRENT >> 8) & 0xFF;
    TxData[1] = LIMIT_MAX_AC_CURRENT & 0xFF;
    Send_CAN_Message((PACKET_ID_SET_MAX_AC << 8) | NODE_ID, TxData, 2);
    HAL_Delay(10);

    // Max Brake
    TxData[0] = (LIMIT_MAX_BRAKE_CURRENT >> 8) & 0xFF;
    TxData[1] = LIMIT_MAX_BRAKE_CURRENT & 0xFF;
    Send_CAN_Message((PACKET_ID_SET_MAX_BRAKE << 8) | NODE_ID, TxData, 2);
    HAL_Delay(10);

    // --- 2. Inicializar corriente mínima para encender ---
    target_rel_current = 50; // 5% como prueba inicial
    TxData[0] = (target_rel_current >> 8) & 0xFF;
    TxData[1] = target_rel_current & 0xFF;
    Send_CAN_Message((PACKET_ID_SET_REL_CURRENT << 8) | NODE_ID, TxData, 2);
    HAL_Delay(10);

    while (1)
    {
        // --- 3. Leer ADC con refresco garantizado ---
        uint32_t adc_accum = 0;
        const int samples = 10;

        for (int i = 0; i < samples; i++) {
            HAL_ADC_Start(&hadc1); // Iniciamos conversión
            if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
                adc_accum += HAL_ADC_GetValue(&hadc1);
            }
            HAL_ADC_Stop(&hadc1); // Forzamos parada para resetear el registro de datos
        }
        adc_value = adc_accum / samples;

        // --- 4. Mapear ADC a Corriente Relativa ---
        // Añadimos una zona muerta (deadband) del 2% para evitar que el motor
        // zumbe cuando el potenciómetro está al mínimo.
        if (adc_value < 100) {
            target_rel_current = 0;
        } else {
            target_rel_current = map(adc_value, 100, 4095, REL_CURRENT_MIN, REL_CURRENT_MAX);
        }

        // --- 5. Drive Enable (Keep-Alive) ---
        TxData[0] = 0x01;
        Send_CAN_Message((PACKET_ID_DRIVE_ENABLE << 8) | NODE_ID, TxData, 1);

        // Pequeño delay para no saturar el bus CAN si el inversor es lento
        HAL_Delay(5);

        // --- 6. Set Relative Current ---
        TxData[0] = (target_rel_current >> 8) & 0xFF;
        TxData[1] = target_rel_current & 0xFF;
        Send_CAN_Message((PACKET_ID_SET_REL_CURRENT << 8) | NODE_ID, TxData, 2);

        // --- 7. Delay total ---
        HAL_Delay(45); // Frecuencia de actualización de 20Hz total
    }
}

/* System Clock Configuration */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
    RCC_OscInitStruct.PLL.PLLN = 85;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) Error_Handler();
}

/* Send CAN Message */
void Send_CAN_Message(uint32_t id, uint8_t* data, uint32_t len)
{
    TxHeader.Identifier = id;
    switch (len)
    {
        case 0: TxHeader.DataLength = FDCAN_DLC_BYTES_0; break;
        case 1: TxHeader.DataLength = FDCAN_DLC_BYTES_1; break;
        case 2: TxHeader.DataLength = FDCAN_DLC_BYTES_2; break;
        case 3: TxHeader.DataLength = FDCAN_DLC_BYTES_3; break;
        case 4: TxHeader.DataLength = FDCAN_DLC_BYTES_4; break;
        case 5: TxHeader.DataLength = FDCAN_DLC_BYTES_5; break;
        case 6: TxHeader.DataLength = FDCAN_DLC_BYTES_6; break;
        case 7: TxHeader.DataLength = FDCAN_DLC_BYTES_7; break;
        default: TxHeader.DataLength = FDCAN_DLC_BYTES_8; break;
    }

    while (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) == 0) {}

    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, data) != HAL_OK)
    {
        Error_Handler();
    }
}

/* Error Handler */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(200);
    }
}
