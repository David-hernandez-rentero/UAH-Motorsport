/* USER CODE BEGIN Header */
/**
  *****************************************************************************
  * @file           : funcion_utils.c
  * @author         : Guillermo Polo
  * @project        : main
  * @date           : 2026-08-12
  * @editor         : Visual Studio Code
  *
  * @brief          : Main program body
  *****************************************************************************
  */
/* USER CODE END Header */


#include "stm32g4xx_hal.h"                                      // Inclusión de la libreía hall
#include "funcion_utils.h"                                      // Inclusión de la librería de funciones útiles

uint8_t delay(uint32_t *lastTick, uint32_t interval_ms) {       // Función para crear un delay
    uint32_t currentTick = HAL_GetTick();                       // Obtención del contador para iniciar el tiempo
    if ((currentTick - *lastTick) >= interval_ms) {             // Si es menor al tiempo deseado
        *lastTick = currentTick;                                // Aumentamos la cuenta
        return 1;                                               // Continuamos
    }
    return 0;                                                   // Fin del delay
}
