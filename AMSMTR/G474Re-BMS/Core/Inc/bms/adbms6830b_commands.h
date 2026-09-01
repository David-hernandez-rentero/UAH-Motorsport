#ifndef G474RE_BMS_ADBMS6830B_COMMANDS_H
#define G474RE_BMS_ADBMS6830B_COMMANDS_H

#include <stdint.h>

// https://www.analog.com/media/en/technical-documentation/data-sheets/adbms6830b.pdf

// RSTCC
extern uint8_t const Reset_Counter[4];

// Config (CFG)
extern uint8_t const R_Config_A[4];
extern uint8_t const W_Config_A[4];
extern uint8_t const R_Config_B[4];
extern uint8_t const W_Config_B[4];

// SNAP/UNSNAP
extern uint8_t const SNAP[4];
extern uint8_t const UNSNAP[4];

// CADC
extern uint8_t const ADCV_CONT[4];
extern uint8_t const R_AvgCell_A[4];
extern uint8_t const R_AvgCell_B[4];
extern uint8_t const R_AvgCell_C[4];
extern uint8_t const R_AvgCell_D[4];
extern uint8_t const R_AvgCell_E[4];
extern uint8_t const R_AvgCell_F[4];

// AUX
extern uint8_t const ADAX[4];
extern uint8_t const PL_AUX[4];
extern uint8_t const R_AUX_C[4];

#endif // G474RE_BMS_ADBMS6830B_COMMANDS_H
