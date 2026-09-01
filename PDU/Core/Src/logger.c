/* USER CODE BEGIN Header */
/**
  *****************************************************************************
  * @file           : logger.c
  * @author         : Guillermo Polo
  * @project        : main
  * @date           : 2026-08-12
  * @editor         : Visual Studio Code
  *
  * @brief          : Main program body
  *****************************************************************************
  */
/* USER CODE END Header */

#include "logger.h"                                                                                                     // Incluye la interfaz pública del módulo de registro.
#include "main.h"                                                                                                       // Permite utilizar funciones HAL como HAL_GetTick().
#include <stdio.h>                                                                                                      // Proporciona printf() y vsnprintf() para formar y mostrar los mensajes.
#include <stdarg.h>                                                                                                     // Proporciona el manejo de argumentos variables utilizado por log_write().

// Nivel mínimo visible — cambiar a LOG_LEVEL_INFO para ocultar DEBUG
volatile LogLevel_t log_min_level = LOG_LEVEL_DEBUG;                                                                    // Define DEBUG como nivel mínimo, por lo que inicialmente se muestran todos los mensajes.

static const char* level_str(LogLevel_t level) {                                                                        // Convierte el nivel enumerado del logger en una etiqueta textual abreviada.
    switch (level) {
        case LOG_LEVEL_DEBUG: return "DBG";                                                                             // Asocia el nivel de depuración con la etiqueta DBG.
        case LOG_LEVEL_INFO:  return "INF";                                                                             // Asocia el nivel informativo con la etiqueta INF.
        case LOG_LEVEL_WARN:  return "WRN";                                                                             // Asocia el nivel de advertencia con la etiqueta WRN.
        case LOG_LEVEL_ERROR: return "ERR";                                                                             // Asocia el nivel de error con la etiqueta ERR.
        default:              return "???";                                                                             // Devuelve una etiqueta genérica si el nivel recibido no está definido.
    }
}

void log_write(LogLevel_t level, const char *fmt, ...) {                                                                // Genera una entrada de log a partir de un nivel y una cadena de formato.
    if (level < log_min_level) return;                                                                                  // Descarta los mensajes cuyo nivel sea inferior al umbral configurado.

    char line[128];                                                                                                     // Reserva un buffer local para construir el texto final del mensaje.
    va_list args;                                                                                                       // Declara la estructura necesaria para recorrer los argumentos variables.
    va_start(args, fmt);                                                                                                // Inicializa la lectura de argumentos adicionales a partir de fmt.
    vsnprintf(line, sizeof(line), fmt, args);                                                                           // Formatea de manera acotada los argumentos dentro del buffer local.
    va_end(args);                                                                                                       // Finaliza el procesamiento de la lista de argumentos variables.

    printf("[%8lu][%s] %s", HAL_GetTick(), level_str(level), line);                                                     // Muestra timestamp, nivel y contenido del mensaje mediante la salida asociada a printf().
}
