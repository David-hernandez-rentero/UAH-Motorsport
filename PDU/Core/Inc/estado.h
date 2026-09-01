//
// Created by guill on 30/09/2025.
//
#ifndef ESTADO_H
#define ESTADO_H

#include "main.h"  // Para los tipos de datos HAL y pines

// ===========================================================================
// ESTADOS DEL SISTEMA R2D
// ===========================================================================
typedef enum
{
    ESPERA        = 0x00,
    ARRANQUE      = 0x01,
    PRECARGA      = 0x02,
    VERIFICACION  = 0x03,
    FUNCIONANDO   = 0x04
} Estado_t;

// ===========================================================================
// VARIABLES GLOBALES DE ESTADO
// ===========================================================================
extern Estado_t estado;
extern uint8_t estado_seguro;

extern volatile uint32_t voltaje_inversor_mV;
extern volatile uint32_t voltaje_bateria_mV;

// Subfase interna de la secuencia de precarga.
// Se declara aquí para que pueda ser leída desde fdcan.c.
extern uint8_t fase;

// Código de error transmitido en R2D_ErrorStatus (bytes 6 y 7 del mensaje CAN).
extern uint16_t codigo_error;

// ===========================================================================
// VARIABLES DE DEPURACIÓN / FEEDBACK
// ===========================================================================
extern volatile uint8_t dbg_air_high;
extern volatile uint8_t dbg_air_low;
extern volatile uint8_t dbg_precharge;
extern volatile uint8_t dbg_fase;
extern volatile uint8_t dbg_estado;

// No se declara dbg_discharge porque el HW actual no dispone de feedback
// físico para el relé de descarga.

// ===========================================================================
// CÓDIGOS DE VERIFICACIÓN
// ===========================================================================
typedef enum
{
    VERIF_OK,
    VERIF_FALLO_DESCARGA,
    VERIF_FALLO_AIR,
    READY_TO_DRIVE,
    VERIF_FALLO_DESCARGA_FINAL,
    MENSAJE_ECU
} CodigoVerificacion;

// ===========================================================================
// PROTOTIPOS
// ===========================================================================
void verificar_estado_LV(void);
void reinicio_sistema(void);
void estado_inicial_HV(void);
void estado_espera(void);
void estado_arranque(void);
void estado_precarga(void);
void estado_funcionando(void);
void estado_maquina(void);
void estado_verificacion(void);
void faseSegura(void);
void faseInsegura(void);

#endif // ESTADO_H
