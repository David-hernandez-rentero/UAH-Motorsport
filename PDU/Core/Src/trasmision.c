/* USER CODE BEGIN Header */
/**
  *****************************************************************************
  * @file           : trasmision.c
  * @author         : Guillermo Polo
  * @author         : David Hernández Rentero
  * @project        : main
  * @date           : 2026-08-12
  * @editor         : Visual Studio Code
  *
  * @brief          : Main program body
  *****************************************************************************
  */
/* USER CODE END Header */


#include "trasmision.h"                                                                                                 // Incluye las funciones públicas asociadas al módulo de transmisión y verificación.
#include <stdbool.h>                                                                                                    // Permite utilizar variables booleanas para memorizar estados de aviso.
#include <stdio.h>                                                                                                      // Proporciona printf() para emitir mensajes de diagnóstico.
#include "estado.h"                                                                                                     // Importa los estados y códigos de verificación definidos por la máquina de estados.
#define COD_VERIF_NINGUNO 0xFF                                                                                          // Valor reservado para indicar que todavía no se ha emitido ningún código de verificación.

// Definición del array de mensajes (Fuente única de verdad)
static const char *const mensajes_verificacion[] = {                                                                    // Tabla constante que centraliza el texto asociado a cada código de verificación.
    [VERIF_OK]                   = "Funciona correctamente de momento\n",                                               // Asocia el estado de verificación correcta con su mensaje informativo.
    [VERIF_FALLO_DESCARGA]       = "Fallo de descarga\n",                                                               // Asocia el fallo de descarga con su mensaje de diagnóstico.
    [VERIF_FALLO_AIR]            = "Fallo de AIR- o de Precarga\n",                                                     // Asocia el fallo de AIR o precarga con su mensaje de diagnóstico.
    [READY_TO_DRIVE]             = "Esta todo preparado para funcionar correctamente\n",                                // Asocia el estado Ready To Drive con su mensaje de confirmación.
    [VERIF_FALLO_DESCARGA_FINAL] = "Fallo de descarga final, coche inseguro\n",                                         // Asocia el fallo final de descarga con la condición de vehículo inseguro.
    [MENSAJE_ECU]                = "Mandar mensaje por funcionamiento ok de todo el sistema\n"                          // Asocia el código destinado a la ECU con el mensaje previsto para ese evento.
};

void mensajeVerificacion(CodigoVerificacion codigo) {                                                                   // Gestiona la presentación de un mensaje a partir del código de verificación recibido.
    // Variable estática para recordar el último mensaje enviado
    static CodigoVerificacion ultimo_codigo = COD_VERIF_NINGUNO;                                                        // Conserva entre llamadas el último código procesado para evitar mensajes repetidos.
    // Si el código es el mismo que el anterior, salimos sin imprimir
    if (codigo == ultimo_codigo) {                                                                                      // Comprueba si el código actual coincide con el último código mostrado.
        return;                                                                                                         // Finaliza la función cuando no existe un cambio de código.
    }

    // Si es un código nuevo, actualizamos y procesamos el switch
    ultimo_codigo = codigo;                                                                                             // Actualiza el último código antes de procesar el nuevo evento.

    switch (codigo) {                                                                                                   // Selecciona el mensaje correspondiente mediante el código enumerado recibido.
        case VERIF_OK:
            printf("%s", mensajes_verificacion[VERIF_OK]);                                                              // Muestra el texto asociado a una verificación correcta.
            break;
        case VERIF_FALLO_DESCARGA:
            printf("%s", mensajes_verificacion[VERIF_FALLO_DESCARGA]);                                                  // Muestra el texto asociado a un fallo durante la descarga.
            break;
        case VERIF_FALLO_AIR:
            printf("%s", mensajes_verificacion[VERIF_FALLO_AIR]);                                                       // Muestra el texto asociado a un fallo de AIR o del circuito de precarga.
            break;
        case READY_TO_DRIVE:
            printf("%s", mensajes_verificacion[READY_TO_DRIVE]);                                                        // Muestra el mensaje que confirma la condición Ready To Drive.
            break;
        case VERIF_FALLO_DESCARGA_FINAL:
            printf("%s", mensajes_verificacion[VERIF_FALLO_DESCARGA_FINAL]);                                            // Muestra el mensaje asociado a un fallo de descarga final.
            break;
        case MENSAJE_ECU:
            printf("%s", mensajes_verificacion[MENSAJE_ECU]);                                                           // Muestra el mensaje previsto para notificar el estado a la ECU.
            // enviar_mensaje_CAN();
            break;
        default:
            printf("Código de verificación desconocido\n");                                                             // Informa de un código no contemplado por la enumeración.
            break;
    }
}


void verificacionEstadoSeguro(GPIO_PinState btn,uint32_t miliVolts) {                                                   // Comprueba conjuntamente el estado de la entrada y el nivel de tensión recibido.
    static bool aviso_exito_enviado = false;                                                                            // Memoriza si el aviso de condición correcta ya ha sido enviado.
    if (btn == GPIO_PIN_SET && miliVolts > 500) {                                                                       // Valida que el botón esté activo y que la tensión supere el umbral implementado.
        if (!aviso_exito_enviado) {                                                                                     // Evita volver a imprimir el aviso mientras la condición correcta se mantenga.
            printf("Boton pulsado! Voltaje correcto: %lu mV\r\n", miliVolts);                                           // Informa una única vez de que ambas condiciones son correctas.
            aviso_exito_enviado = true;                                                                                 // Registra que el aviso de éxito ya ha sido emitido.
        }
    }
    else {
        if (aviso_exito_enviado) {                                                                                      // Solo genera una alerta si anteriormente se había alcanzado la condición correcta.
            printf("ALERTA: Se ha soltado el boton o ha caido el voltaje (%lu mV)\r\n", miliVolts);                     // Notifica la pérdida del botón o la caída de tensión respecto a la condición anterior.
            aviso_exito_enviado = false;                                                                                // Rearma el aviso para permitir una nueva notificación de éxito posteriormente.
        }
    }
}

void voltajePrecarga(uint32_t miliVolts,int mediciones_realizadas) {                                                    // Muestra por terminal cada medida de tensión realizada durante la precarga.
    printf("Medicion %d: %lu mV\r\n", mediciones_realizadas + 1, miliVolts);                                            // Imprime el número de medida y el valor de tensión recibido por la función.

}