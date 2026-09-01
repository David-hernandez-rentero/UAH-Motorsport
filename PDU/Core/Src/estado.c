/* USER CODE BEGIN Header */
/**
  *****************************************************************************
  * @file           : estado.c
  * @author         : Guillermo Polo
  * @project        : estado
  * @date           : 2026-08-12
  * @editor         : Visual Studio Code
  *
  * @brief          :Program for the state of the PDU
  *****************************************************************************
  */
/* USER CODE END Header */

#include "estado.h"                                                                                                     // Incluye los tipos, estados y prototipos públicos de la máquina de estados.
#include <stdio.h>                                                                                                      // Proporciona printf() para los mensajes de diagnóstico de cada etapa.
#include "fdcan.h"                                                                                                      // Permite acceder a flags y variables actualizadas por la comunicación CAN.
#include "funcion_utils.h"                                                                                              // Incluye la función auxiliar de temporización utilizada por las secuencias.
#include "main.h"                                                                                                       // Proporciona las definiciones de GPIO y recursos generales de la aplicación.

// ===========================================================================
// LÓGICA DE RELÉS — cambiar aquí si la polaridad se invierte
// ===========================================================================
#define RELE_CERRADO GPIO_PIN_SET                                                                                       // Define el nivel lógico utilizado para ordenar el cierre de un relé.
#define RELE_ABIERTO GPIO_PIN_RESET                                                                                     // Define el nivel lógico utilizado para ordenar la apertura de un relé.

// ===========================================================================
// UMBRALES
// Voltaje del inversor en mV escala real (0–400000 mV = 0–400V)
// Llegará por CAN — de momento simulado con sim_voltaje_inversor_mV
// ===========================================================================
#define UMBRAL_PRECARGA_OK_MV   (voltaje_bateria_mV*0.955)     // 95% de 400V
#define TIMEOUT_PRECARGA_MS     2000U       // 2s máximo para cargar el inversor
#define TIMEOUT_DESCARGA_MS     2000U       // 2s para descargar antes de abrir AIR-

// ===========================================================================
// VARIABLES DE ESTADO
// ===========================================================================
Estado_t estado = ESPERA;                                                                                               // Inicializa la máquina de estados en la condición de espera.
uint8_t  estado_seguro = 0;                                                                                             // Indica inicialmente que la condición de seguridad LV todavía no está validada.

static uint32_t startTick = 0;                                                                                          // Almacena el instante de referencia utilizado en transiciones temporizadas.
uint8_t   fase = 0;                                                                                                     // Identifica la subfase actual dentro de la secuencia de precarga.
static uint8_t  verificacion_completada = 0;                                                                            // Evita repetir la verificación final una vez completada correctamente.

volatile uint32_t voltaje_inversor_mV = 0;                                                                              // Valor de tensión del inversor compartido con el módulo de recepción CAN.
volatile uint32_t voltaje_bateria_mV = 0;                                                                               // Valor de tensión de batería compartido con el módulo de recepción CAN.
uint16_t codigo_error = 0;                                                                                              // Almacena el código de error transmitido en R2D_ErrorStatus.

// ===========================================================================
// VARIABLES DEBUG — añadir al watch del debugger
// sim_voltaje_inversor_mV: poner a 390000 para simular precarga OK (llegará por CAN)
// ===========================================================================
volatile uint8_t  dbg_air_high            = 0;                                                                          // Variable de observación que refleja el feedback lógico del AIR+.
volatile uint8_t  dbg_air_low             = 0;                                                                          // Variable de observación que refleja el feedback lógico del AIR-.
volatile uint8_t  dbg_precharge           = 0;                                                                          // Variable de observación que refleja el feedback lógico del relé de precarga.
volatile uint8_t  dbg_fase                = 0;                                                                          // Expone la subfase de precarga actual para facilitar la depuración.
volatile uint8_t  dbg_estado              = 0;                                                                          // Expone el estado principal actual para facilitar la depuración.
volatile uint32_t sim_voltaje_inversor_mV = 0;                                                                          // Variable auxiliar prevista para simular la tensión del inversor durante pruebas.

// NOTA: no existe verificación de discharge (dbg_discharge retirado).
// No hay feedback físico (LM393) para ese relé en el HW actual.

// ===========================================================================
// HELPER — actualiza variables debug
// ===========================================================================
static void actualizar_debug(void) {                                                                                    // Actualiza las variables de depuración a partir de los GPIO y del estado interno.
    // Feedback LM393 activo en LOW:
    // LOW  = relé cerrado
    // HIGH = relé abierto

    dbg_air_high  = (HAL_GPIO_ReadPin(INPUT_AIR_HIGH_GPIO_Port,
                                      INPUT_AIR_HIGH_Pin) == GPIO_PIN_RESET);                                           // Interpreta como relé cerrado el nivel activo bajo del feedback del AIR+.

    dbg_air_low   = (HAL_GPIO_ReadPin(INPUT_AIR_LOW_GPIO_Port,
                                      INPUT_AIR_LOW_Pin) == GPIO_PIN_RESET);                                            // Interpreta como relé cerrado el nivel activo bajo del feedback del AIR-.

    dbg_precharge = (HAL_GPIO_ReadPin(INPUT_PRECHARGE_GPIO_Port,
                                      INPUT_PRECHARGE_Pin) == GPIO_PIN_RESET);                                          // Interpreta como relé cerrado el nivel activo bajo del feedback de precarga.

    dbg_fase   = fase;                                                                                                  // Copia la subfase interna en la variable visible desde el depurador.
    dbg_estado = (uint8_t)estado;                                                                                       // Copia el estado enumerado actual en la variable visible desde el depurador.
}




// ===========================================================================
// HELPER — imprime estado de todos los relés
// ===========================================================================
static void imprimir_estado_reles(void) {                                                                               // Muestra por terminal un resumen del estado lógico de los relés supervisados.
    printf("  [RELES] AIR+=%-3s AIR-=%-3s PREC=%-3s\r\n",                                                               // Construye el mensaje textual que representa AIR+, AIR- y precarga.
           dbg_air_high  ? "ON" : "OFF",
           dbg_air_low   ? "ON" : "OFF",
           dbg_precharge ? "ON" : "OFF");
}

// ===========================================================================
// DESCARGA SEGURA
// 1. Abrir AIR+
// 2. Esperar 2s
// 3. Abrir AIR-
// (Discharge se comanda pero no se verifica: sin feedback físico disponible)
// ===========================================================================
static void secuencia_descarga(void) {                                                                                  // Ejecuta la secuencia utilizada para llevar los AIR a una condición de descarga segura.
    uint32_t ahora = HAL_GetTick();                                                                                     // Obtiene una referencia temporal para la espera implementada en la secuencia.

    printf("[DESCARGA] Iniciando descarga segura del inversor\r\n");

    printf("[DESCARGA] Paso 1: Abriendo AIR+\r\n");
    HAL_GPIO_WritePin(OUTPUT_AIR_HIGH_GPIO_Port, OUTPUT_AIR_HIGH_Pin, RELE_ABIERTO);                                    // Ordena la apertura del AIR+ como primer paso de la descarga segura.
    actualizar_debug();                                                                                                 // Actualiza las señales de feedback tras el cambio de mando.
    imprimir_estado_reles();                                                                                            // Muestra el estado observado de los relés después de abrir AIR+.

    printf("[DESCARGA] Paso 2: Esperando 2s...\r\n");
    delay(&ahora, TIMEOUT_DESCARGA_MS);                                                                                 // Ejecuta la espera configurada antes de abrir el AIR-.

    printf("[DESCARGA] Paso 3: Abriendo AIR-\r\n");
    HAL_GPIO_WritePin(OUTPUT_AIR_LOW_GPIO_Port, OUTPUT_AIR_LOW_Pin, RELE_ABIERTO);                                      // Ordena la apertura del AIR- tras el intervalo de descarga.
    actualizar_debug();                                                                                                 // Actualiza los feedbacks después de actuar sobre el AIR-.
    imprimir_estado_reles();                                                                                            // Muestra el estado final observado de los relés.

    printf("[DESCARGA] Descarga completada\r\n");
}

// ===========================================================================
// REINICIO DEL SISTEMA
// ===========================================================================
void reinicio_sistema(void) {                                                                                           // Reinicia la lógica del sistema y devuelve la máquina al estado ESPERA.
    static uint8_t reiniciando = 0;                                                                                     // Flag estática utilizada para impedir llamadas recursivas al proceso de reinicio.
    if (reiniciando) return;                                                                                            // Abandona la función si ya existe un reinicio en curso.
    reiniciando = 1;                                                                                                    // Bloquea nuevas solicitudes de reinicio hasta completar la secuencia actual.

    printf("\r\n[REINICIO] ---- Iniciando reinicio del sistema ----\r\n");

    estado_seguro           = 0;                                                                                        // Invalida la condición de seguridad previamente almacenada.
    fase                    = 0;                                                                                        // Reinicia la subfase de la secuencia de precarga.
    startTick               = 0;                                                                                        // Elimina la referencia temporal de la secuencia anterior.
    verificacion_completada = 0;                                                                                        // Permite que la verificación final pueda ejecutarse de nuevo.
    sim_voltaje_inversor_mV = 0;                                                                                        // Reinicia la variable de simulación del voltaje del inversor.
    can_ok                  = 0;                                                                                        // Descarta cualquier orden CAN de activación que estuviese pendiente.

    secuencia_descarga();                                                                                               // Ejecuta la apertura ordenada de los AIR antes de restablecer el sistema.

    HAL_GPIO_WritePin(OUTPUT_PRECHARGE_GPIO_Port, OUTPUT_PRECHARGE_Pin, RELE_ABIERTO);                                  // Garantiza que el relé de precarga quede comandado en estado abierto.
    HAL_GPIO_WritePin(OUTPUT_DISCHARGE_GPIO_Port, OUTPUT_DISCHARGE_Pin, RELE_ABIERTO);                                  // Ordena también la apertura del relé de descarga al finalizar el reinicio.

    estado = ESPERA;                                                                                                    // Devuelve explícitamente la máquina de estados a ESPERA.

    HAL_GPIO_WritePin(LED_AVISO2_GPIO_Port, LED_AVISO2_Pin, GPIO_PIN_SET);                                              // Activa el LED empleado como aviso durante la condición de reinicio/espera.
    HAL_GPIO_WritePin(LED_AVISO1_GPIO_Port, LED_AVISO1_Pin, GPIO_PIN_RESET);                                            // Desactiva el LED asociado a la activación de la secuencia.

    actualizar_debug();                                                                                                 // Actualiza las variables de depuración con la nueva condición.
    printf("[REINICIO] Sistema en ESPERA\r\n");
    imprimir_estado_reles();                                                                                            // Muestra por terminal el estado de los relés tras el reinicio.

    reiniciando = 0;                                                                                                    // Libera el bloqueo para permitir futuros reinicios.
}

// ===========================================================================
// ESTADO INICIAL HV
// ===========================================================================
void estado_inicial_HV(void) {                                                                                          // Establece la posición de los relés al iniciar el sistema de alta tensión.
    printf("[INIT] Inicializando relés HV — todos abiertos\r\n");
    HAL_GPIO_WritePin(OUTPUT_PRECHARGE_GPIO_Port, OUTPUT_PRECHARGE_Pin, RELE_ABIERTO);                                  // Mantiene abierto el relé de precarga durante la inicialización.
    HAL_GPIO_WritePin(OUTPUT_AIR_HIGH_GPIO_Port,  OUTPUT_AIR_HIGH_Pin,  RELE_ABIERTO);                                  // Mantiene abierto el AIR+ durante la inicialización.
    HAL_GPIO_WritePin(OUTPUT_AIR_LOW_GPIO_Port,   OUTPUT_AIR_LOW_Pin,   RELE_ABIERTO);                                  // Mantiene abierto el AIR- durante la inicialización.
    HAL_GPIO_WritePin(OUTPUT_DISCHARGE_GPIO_Port, OUTPUT_DISCHARGE_Pin, RELE_CERRADO);                                  // Relé normalmente cerrado
    actualizar_debug();                                                                                                 // Actualiza las variables de feedback después de establecer el estado inicial.
    imprimir_estado_reles();                                                                                            // Muestra por terminal el estado inicial observado.
}

// ===========================================================================
// VERIFICACIÓN LV
// ===========================================================================
void verificar_estado_LV(void) {                                                                                        // Evalúa la entrada del circuito de seguridad de baja tensión.
    GPIO_PinState SC_state = HAL_GPIO_ReadPin(INPUT_SC_GPIO_Port, INPUT_SC_Pin);                                        // Lee el nivel lógico presente en la entrada INPUT_SC.

    if (SC_state == GPIO_PIN_SET) {                                                                                     // Comprueba si el circuito de seguridad se encuentra en el estado considerado correcto.
        estado_seguro = 1;                                                                                              // Marca como válida la condición de seguridad LV.
        HAL_GPIO_WritePin(LED_AVISO2_GPIO_Port, LED_AVISO2_Pin, GPIO_PIN_RESET);                                        // Actualiza el LED de aviso para reflejar una condición segura.
        printf("[LV] Sistema seguro OK\r\n");
    } else {
        estado_seguro = 0;                                                                                              // Invalida la condición de seguridad cuando la entrada no es correcta.
        printf("[LV] FALLO seguridad — reiniciando\r\n");
        reinicio_sistema();                                                                                             // Inicia el procedimiento de reinicio y descarga ante un fallo de seguridad.
    }
}

// ===========================================================================
// ESTADO: ESPERA
// Único disparador: mensaje CAN de activación OK (can_ok, ID 0x0000301B) ,
// puesto por HAL_FDCAN_RxFifo0Callback en fdcan.c).
// ===========================================================================
void estado_espera(void) {                                                                                              // Gestiona el estado de espera previo al inicio de la secuencia Ready To Drive.
    if (!estado_seguro) return;                                                                                         // Impide abandonar ESPERA mientras no se haya validado la seguridad LV.

    if (can_ok) {                                                                                                       // Comprueba si se ha recibido por CAN la orden de activación.
        can_ok = 0;                                                                                                     // Consume la flag CAN para que la orden se procese una sola vez.
        estado = ARRANQUE;                                                                                              // Avanza la máquina de estados desde ESPERA hasta ARRANQUE.
        HAL_GPIO_WritePin(LED_AVISO1_GPIO_Port, LED_AVISO1_Pin, GPIO_PIN_SET);                                          // Activa el LED que indica la recepción de la orden de arranque.
        startTick = HAL_GetTick();   // <- añade esta línea si no la tienes
        printf("[ESPERA] CAN OK recibido -> ARRANQUE\r\n");                                                             // Informa por terminal de la transición provocada por la orden CAN.
    }
}

// ===========================================================================
// ESTADO: ARRANQUE
// Antes esperaba a que terminase el pitido del buzzer (ya retirado).
// Ahora espera un tiempo fijo tras recibir el CAN OK antes de iniciar precarga.
// ===========================================================================
#define TIEMPO_ARRANQUE_MS 500U   // ajusta el valor que decidáis en el equipo

void estado_arranque(void) {                                                                                            // Gestiona la espera temporal entre la orden CAN y el inicio de la precarga.
    if (HAL_GetTick() - startTick >= TIEMPO_ARRANQUE_MS) {                                                              // Comprueba si ha transcurrido el tiempo de arranque configurado.
        fase = 0;                                                                                                       // Asegura que la precarga comience desde su primera subfase.
        startTick = 0;                                                                                                  // Limpia la referencia temporal utilizada durante ARRANQUE.
        estado = PRECARGA;                                                                                              // Avanza la máquina al estado PRECARGA.
        printf("[ARRANQUE] Tiempo cumplido → PRECARGA\r\n");
    }
}

// ===========================================================================
// ESTADO: PRECARGA
//
// Feedbacks leídos directamente de GPIO (señal LED via LM393)
// sim_voltaje_inversor_mV simula el voltaje del inversor (llegará por CAN)
//
// Secuencia:
//   Fase 0 → Abrir Discharge (sin verificación, no hay feedback físico)
//   Fase 1 → Cerrar AIR-       + verificar feedback
//   Fase 2 → Cerrar Precarga   + verificar feedback
//   Fase 3 → Esperar voltaje >= 380V en max 2s
//   Fase 4 → Cerrar AIR+       + verificar feedback
//   Fase 5 → Abrir Precarga    + verificar feedback
//   Fase 6 → Abrir Discharge (sin verificación) → VERIFICACION
// ===========================================================================
void estado_precarga(void) {                                                                                            // Ejecuta secuencialmente las distintas subfases del proceso de precarga.

    uint32_t ahora = HAL_GetTick();                                                                                     // Obtiene el tick actual utilizado para temporizar las operaciones de esta iteración.
    uint32_t voltaje = 0;                                                                                               // Reserva la variable local que contendrá la tensión de inversor evaluada.

    switch (fase)                                                                                                       // Selecciona la acción de precarga en función de la subfase activa.
    {
        case 0:
            printf("\r\n[PRECARGA] ---- Iniciando secuencia de precarga ----\r\n");
            printf("[PRECARGA] Fase 0: Abriendo Discharge (sin feedback disponible)\r\n");
            HAL_GPIO_WritePin(OUTPUT_DISCHARGE_GPIO_Port, OUTPUT_DISCHARGE_Pin, RELE_ABIERTO);                          // Ordena abrir el relé de descarga antes de cerrar el circuito de precarga.
            delay(&ahora, 50);                                                                                          // Introduce el retardo implementado antes de leer los estados de feedback.
            actualizar_debug();                                                                                         // Actualiza las variables de feedback después de la orden de apertura.

            printf("[PRECARGA] Fase 0: Discharge comandado (no verificable)\r\n");
            imprimir_estado_reles();
            startTick = ahora;                                                                                          // Guarda el instante actual como referencia de la siguiente subfase.
            fase = 1;                                                                                                   // Avanza desde la fase 0 hasta la fase 1.
            break;

        case 1:
            printf("[PRECARGA] Fase 1: Cerrando AIR-\r\n");
            HAL_GPIO_WritePin(OUTPUT_AIR_LOW_GPIO_Port, OUTPUT_AIR_LOW_Pin, RELE_CERRADO);                              // Ordena el cierre del AIR- como primer contactor principal de la secuencia.
            delay(&ahora,50);                                                                                           // Aplica el retardo previo a la comprobación de feedback del AIR-.
            actualizar_debug();                                                                                         // Actualiza el estado observado de los relés.

            if (!dbg_air_low) {                                                                                         // Comprueba que el feedback confirme el cierre del AIR-.
                printf("[PRECARGA] FALLO fase 1: AIR- no ha cerrado (dbg=%d)\r\n", dbg_air_low);
                imprimir_estado_reles();
                reinicio_sistema();                                                                                     // Aborta la secuencia y reinicia el sistema si AIR- no confirma el cierre.
                break;
            }
            printf("[PRECARGA] Fase 1 OK: AIR- cerrado\r\n");
            imprimir_estado_reles();
            startTick = ahora;                                                                                          // Actualiza la referencia temporal para la siguiente operación.
            fase = 2;                                                                                                   // Avanza a la fase de cierre del relé de precarga.
            break;

        case 2:
            printf("[PRECARGA] Fase 2: Cerrando Precarga\r\n");
            HAL_GPIO_WritePin(OUTPUT_PRECHARGE_GPIO_Port, OUTPUT_PRECHARGE_Pin, RELE_CERRADO);                          // Ordena cerrar el relé de precarga.
            delay(&ahora,50);                                                                                           // Aplica el retardo previo a verificar el feedback de precarga.
            actualizar_debug();                                                                                         // Actualiza las variables de feedback después del cierre.

            if (!dbg_precharge) {                                                                                       // Comprueba que el feedback confirme que el relé de precarga está cerrado.
                printf("[PRECARGA] FALLO fase 2: Precarga no ha cerrado (dbg=%d)\r\n", dbg_precharge);
                imprimir_estado_reles();
                reinicio_sistema();                                                                                     // Inicia el reinicio seguro si la precarga no confirma el cierre.
                break;
            }
            printf("[PRECARGA] Fase 2 OK: Precarga cerrada. Esperando voltaje >= 380V\r\n");
            printf("[PRECARGA] (debugger: poner sim_voltaje_inversor_mV = 390000)\r\n");
            imprimir_estado_reles();
            startTick = ahora;                                                                                          // Guarda el instante de inicio de la espera de tensión.
            fase = 3;                                                                                                   // Avanza a la fase de comprobación de tensión del inversor.
            break;

        case 3:
            voltaje = voltaje_inversor_mV;                                                                              // Toma el último valor de tensión del inversor actualizado por CAN.

            if (ahora - startTick >= TIMEOUT_PRECARGA_MS) {                                                             // Espera hasta alcanzar el tiempo máximo previsto para evaluar la precarga.
                if (voltaje >= UMBRAL_PRECARGA_OK_MV) {                                                                 // Compara la tensión del inversor con el porcentaje definido de la tensión de batería.
                    printf("[PRECARGA] Fase 3 OK: Voltaje = %lu V tras 2s\r\n", voltaje / 1000);
                    startTick = ahora;                                                                                  // Actualiza la referencia temporal al validar correctamente la tensión.
                    fase = 4;                                                                                           // Avanza a la fase de cierre del AIR+.
                } else {
                    printf("[PRECARGA] FALLO fase 3: Voltaje insuficiente tras 2s (%lu V)\r\n", voltaje / 1000);
                    reinicio_sistema();                                                                                 // Ejecuta el reinicio seguro cuando la tensión no alcanza el umbral requerido.
                }
            }
            break;

        case 4:
            printf("[PRECARGA] Fase 4: Cerrando AIR+\r\n");
            HAL_GPIO_WritePin(OUTPUT_AIR_HIGH_GPIO_Port, OUTPUT_AIR_HIGH_Pin, RELE_CERRADO);                            // Ordena cerrar el AIR+ una vez validada la precarga.
            delay(&ahora,50);                                                                                           // Aplica el retardo previo a comprobar su feedback.
            actualizar_debug();                                                                                         // Actualiza el estado observado tras el cierre del AIR+.

            if (!dbg_air_high) {                                                                                        // Comprueba que el feedback confirme el cierre del AIR+.
                printf("[PRECARGA] FALLO fase 4: AIR+ no ha cerrado (dbg=%d)\r\n", dbg_air_high);
                imprimir_estado_reles();
                reinicio_sistema();                                                                                     // Reinicia el sistema si el AIR+ no alcanza el estado esperado.
                break;
            }
            printf("[PRECARGA] Fase 4 OK: AIR+ cerrado. Abriendo Precarga\r\n");
            imprimir_estado_reles();
            startTick = ahora;                                                                                          // Actualiza la referencia temporal para la siguiente fase.
            fase = 5;                                                                                                   // Avanza a la fase de apertura del relé de precarga.
            break;

        case 5:
            printf("[PRECARGA] Fase 5: Abriendo Precarga\r\n");
            HAL_GPIO_WritePin(OUTPUT_PRECHARGE_GPIO_Port, OUTPUT_PRECHARGE_Pin, RELE_ABIERTO);                          // Ordena abrir el relé de precarga una vez cerrados ambos AIR.
            delay(&ahora,50);                                                                                           // Aplica el retardo previo a comprobar la apertura.
            actualizar_debug();                                                                                         // Actualiza el feedback de los relés.

            if (dbg_precharge) {                                                                                        // Detecta como fallo que el feedback continúe indicando precarga cerrada.
                printf("[PRECARGA] FALLO fase 5: Precarga no se ha abierto (dbg=%d)\r\n", dbg_precharge);
                imprimir_estado_reles();
                reinicio_sistema();                                                                                     // Reinicia el sistema si no se confirma la apertura del relé de precarga.
                break;
            }
            printf("[PRECARGA] Fase 5 OK: Precarga abierta\r\n");
            imprimir_estado_reles();
            startTick = ahora;                                                                                          // Actualiza la referencia temporal de la secuencia.
            fase = 6;                                                                                                   // Avanza a la última subfase de precarga.
            break;

        case 6:
            printf("[PRECARGA] Fase 6: Abriendo Discharge (sin feedback disponible)\r\n");
            HAL_GPIO_WritePin(OUTPUT_DISCHARGE_GPIO_Port, OUTPUT_DISCHARGE_Pin, RELE_ABIERTO);                          // Ordena mantener abierto el relé de descarga al finalizar la secuencia.
            delay(&ahora,50);                                                                                           // Aplica el retardo implementado antes de completar la fase.
            actualizar_debug();                                                                                         // Actualiza los feedbacks finales.
            imprimir_estado_reles();
            printf("[PRECARGA] Secuencia completada → VERIFICACION\r\n");
            fase = 0;                                                                                                   // Reinicia el índice de subfase para una futura ejecución.
            startTick = 0;                                                                                              // Limpia la referencia temporal de precarga.
            estado = VERIFICACION;                                                                                      // Avanza al estado VERIFICACION para comprobar la configuración final.
            break;

        default:
            fase = 0;                                                                                                   // Recupera la primera subfase si fase contiene un valor no contemplado.
            break;
    }
}

// ===========================================================================
// ESTADO: VERIFICACION
// AIR+ cerrado, AIR- cerrado, Precarga abierta
// ===========================================================================
void estado_verificacion(void) {                                                                                        // Comprueba una única vez la configuración final de contactores tras la precarga.
    if (verificacion_completada) return;                                                                                // Evita repetir la verificación si ya se completó correctamente.

    actualizar_debug();                                                                                                 // Actualiza los feedbacks antes de evaluar la condición final.
    printf("\r\n[VERIFICACION] Comprobando estado final\r\n");
    imprimir_estado_reles();

    if (dbg_air_high == 1 &&                                                                                            // Inicia la comprobación conjunta de AIR+, AIR- y relé de precarga.
        dbg_air_low  == 1 &&
        dbg_precharge == 0)
    {
        printf("[VERIFICACION] OK → Ready To Drive\r\n");
        verificacion_completada = 1;                                                                                    // Marca la verificación como completada para impedir nuevas ejecuciones.
        faseSegura();                                                                                                   // Confirma la condición segura y entra en Ready To Drive.
    } else {
        printf("[VERIFICACION] FALLO: estado incorrecto\r\n");
        faseInsegura();                                                                                                 // Deriva a la rutina de condición insegura si algún feedback no coincide.
    }
}

void faseSegura(void) {                                                                                                 // Gestiona la transición correspondiente a una verificación final correcta.
    printf("\r\n[SISTEMA] ===== READY TO DRIVE =====\r\n");
    estado = FUNCIONANDO;                                                                                               // Sitúa la máquina de estados en FUNCIONANDO.
}

void faseInsegura(void) {                                                                                               // Gestiona una verificación final incorrecta.
    printf("[SISTEMA] FALLO CRITICO — iniciando descarga segura\r\n");
    reinicio_sistema();                                                                                                 // Ejecuta el reinicio seguro ante una condición final no válida.
}

// ===========================================================================
// ESTADO: FUNCIONANDO
// ===========================================================================
void estado_funcionando(void)                                                                                           // Supervisa continuamente las condiciones críticas mientras el sistema está funcionando.
{
    actualizar_debug();                                                                                                 // Actualiza los feedbacks disponibles antes de efectuar las comprobaciones.

    if (!estado_seguro)                                                                                                 // Comprueba que la condición de seguridad LV continúe vigente.
    {
        printf("[FUNCIONANDO] FALLO LV\r\n");
        reinicio_sistema();                                                                                             // Inicia un reinicio seguro si se pierde la condición LV.
        return;
    }

    if (HAL_GPIO_ReadPin(                                                                                               // Lee directamente el feedback del AIR+ para verificar que permanece cerrado.
            INPUT_AIR_HIGH_GPIO_Port,
            INPUT_AIR_HIGH_Pin) != GPIO_PIN_RESET)
    {
        printf("[FUNCIONANDO] FALLO AIR+\r\n");
        reinicio_sistema();                                                                                             // Inicia el reinicio si el AIR+ deja de presentar el estado esperado.
        return;
    }

    if (HAL_GPIO_ReadPin(                                                                                               // Lee directamente el feedback del AIR- para verificar que permanece cerrado.
            INPUT_AIR_LOW_GPIO_Port,
            INPUT_AIR_LOW_Pin) != GPIO_PIN_RESET)
    {
        printf("[FUNCIONANDO] FALLO AIR-\r\n");
        reinicio_sistema();                                                                                             // Inicia el reinicio si el AIR- deja de presentar el estado esperado.
        return;
    }

}

// ===========================================================================
// MÁQUINA DE ESTADOS
// ===========================================================================
void estado_maquina(void) {                                                                                             // Ejecuta la función asociada al estado principal activo se realciona con el DBC en R2D_STATUS. 
    switch (estado) {                                                                                                   // Selecciona la rutina correspondiente mediante la enumeración Estado_t. 
        case ESPERA:        estado_espera();        break;                                                              // Ejecuta la lógica del estado ESPERA. (IDLE)
        case ARRANQUE:      estado_arranque();      break;                                                              // Ejecuta la temporización del estado ARRANQUE.(PROCESS)
        case PRECARGA:      estado_precarga();      break;                                                              // Ejecuta la secuencia de precarga.(CHARGE)
        case VERIFICACION:  estado_verificacion();  break;                                                              // Ejecuta la verificación final de los contactores.(VERIFICATION)
        case FUNCIONANDO:   estado_funcionando();   break;                                                              // Ejecuta la supervisión continua del estado FUNCIONANDO.(ACTIVE)
        default:                                    break;
    }

    actualizar_debug();                                                                                                 // Actualiza las variables de depuración al finalizar cada iteración de la máquina de estados.
}
