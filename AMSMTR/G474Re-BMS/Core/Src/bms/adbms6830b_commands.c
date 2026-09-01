#include "bms/adbms6830b_commands.h"

/**
 * RSTCC
 */
uint8_t const Reset_Counter[4] = {
    // [00000] | [Command = 000 | 00101110]
    0x00, 0x2E,

    // [Command PEC 15] | [0]
    0xC4, 0xC6

    // Precomputed PEC, shifted one bit to the left:
    // https://emn178.github.io/online-tools/crc/?input=002E&source=text&input_type=hex&output_type=hex_upper&model=custom&width=15&poly=C599&init=10&xorout=0&refin=0&refout=0
    // python -c "print(hex(0x6263 << 1))"
};

/**
 * RDCFGA
 */
uint8_t const R_Config_A[4] = {
    // [00000] | [Command = 000 | 00000010]
    0x00, 0x02,

    // [Command PEC 15] | [0]
    0x2B, 0x0A

    // Precomputed PEC, shifted one bit to the left:
    // https://emn178.github.io/online-tools/crc/?input=0002&source=text&input_type=hex&output_type=hex_upper&model=custom&width=15&poly=C599&init=10&xorout=0&refin=0&refout=0
    // python -c "print(hex(0x1585 << 1))"
};
/**
 * WRCFGA
 */
uint8_t const W_Config_A[4] = {
    // [00000] | [Command = 000 | 00000001]
    0x00, 0x01,

    // [Command PEC 15] | [0]
    0x3D, 0x6E

    // Precomputed PEC, shifted one bit to the left:
    // https://emn178.github.io/online-tools/crc/?input=0001&source=text&input_type=hex&output_type=hex_upper&model=custom&width=15&poly=C599&init=10&xorout=0&refin=0&refout=0
    // python -c "print(hex(0x1EB7 << 1))"
};
/**
 * RDCFGB
 */
uint8_t const R_Config_B[4] = {
    // [00000] | [Command = 000 | 00100110]
    0x00, 0x26,

    // [Command PEC 15] | [0]
    0x2C, 0xC8

    // Precomputed PEC, shifted one bit to the left:
    // https://emn178.github.io/online-tools/crc/?input=0026&source=text&input_type=hex&output_type=hex_upper&model=custom&width=15&poly=C599&init=10&xorout=0&refin=0&refout=0
    // python -c "print(hex(0x1664 << 1))"
};
/**
 * WRCFGB
 */
uint8_t const W_Config_B[4] = {
    // [00000] | [Command = 000 | 00100100]
    0x00, 0x24,

    // [Command PEC 15] | [0]
    0xB1, 0x9E

    // Precomputed PEC, shifted one bit to the left:
    // https://emn178.github.io/online-tools/crc/?input=0024&source=text&input_type=hex&output_type=hex_upper&model=custom&width=15&poly=C599&init=10&xorout=0&refin=0&refout=0
    // python -c "print(hex(0x58CF << 1))"
};

uint8_t const SNAP[] = {
    // [00000] | [Command = 000 | 00101101]
    0x00, 0x2D,

    // [Command PEC 15] | [0]
    0xD2, 0xA2

    // Precomputed PEC, shifted one bit to the left:
    // https://emn178.github.io/online-tools/crc/?input=002D&source=text&input_type=hex&output_type=hex_upper&model=custom&width=15&poly=C599&init=10&xorout=0&refin=0&refout=0
    // python -c "print(hex(0x6951 << 1))"
};
uint8_t const UNSNAP[] = {
    // [00000] | [Command = 000 | 00101111]
    0x00, 0x2F,

    // [Command PEC 15] | [0]
    0x4F, 0xF4

    // Precomputed PEC, shifted one bit to the left:
    // https://emn178.github.io/online-tools/crc/?input=002F&source=text&input_type=hex&output_type=hex_upper&model=custom&width=15&poly=C599&init=10&xorout=0&refin=0&refout=0
    // python -c "print(hex(0x27FA << 1))"
};

/**
 * Data Sheet pages (Rev. 0, from 1 to 83): 19-20
 *
 * Command: ADCV (Read an input range of -2V to +5.5V, the average recomputed
 *          every 8ms)
 * - Bit 10: 0
 * - Bit 09: 1
 * - Bit 08: RD = false (only start reading of C-ADCs, no need to also read
 *           the ReDundant S-ADCs which only read an input range of 0V to
 *           +5.5V)
 * - Bit 07: CONT = true (keep reading and recomputing CONTinuously,
 *           not just once)
 * - Bit 06: 1
 * - Bit 05: 1
 * - Bit 04: DCP = false (must be false if CONT = true)
 * - Bit 03: 0
 * - Bit 02: RSTF = true (reset the IIR filters to the default level without
 *           needing to provide a CLRFC command)
 * - Bit 01 and 00: OW = 0 (pag 21 Open Wire switches and pag 24 Open Wire
 *          detection, we can detect an open wire since we have 2
 *          measurements: C-ADCs and S-ADCs)
 */
// TODO NU does an OW fault check periodically
uint8_t const ADCV_CONT[] = {
    // [00000] | [Command = 010 | 11100100]
    0x02, 0xE4,

    // [Command PEC 15] | [0]
    0x89, 0x98

    // Precomputed PEC, shifted one bit to the left:
    // https://emn178.github.io/online-tools/crc/?input=02E4&source=text&input_type=hex&output_type=hex_upper&model=custom&width=15&poly=C599&init=10&xorout=0&refin=0&refout=0
    // python -c "print(hex(0x44CC << 1))"
};
/**
 * RDACA
 */
uint8_t const R_AvgCell_A[] = {
    // [00000] | [Command = 000 | 01000100]
    0x00, 0x44,

    // [Command PEC 15] | [0]
    0xE0, 0x48

    // Precomputed PEC, shifted one bit to the left:
    // https://emn178.github.io/online-tools/crc/?input=0044&source=text&input_type=hex&output_type=hex_upper&model=custom&width=15&poly=C599&init=10&xorout=0&refin=0&refout=0
    // python -c "print(hex(0x7024 << 1))"
};
/**
 * RDACB
 */
uint8_t const R_AvgCell_B[] = {
    // [00000] | [Command = 000 | 01000110]
    0x00, 0x46,

    // [Command PEC 15] | [0]
    0x7D, 0x1E

    // Precomputed PEC, shifted one bit to the left:
    // https://emn178.github.io/online-tools/crc/?input=0046&source=text&input_type=hex&output_type=hex_upper&model=custom&width=15&poly=C599&init=10&xorout=0&refin=0&refout=0
    // python -c "print(hex(0x3E8F << 1))"
};
/**
 * RDACC
 */
uint8_t const R_AvgCell_C[] = {
    // [00000] | [Command = 000 | 01001000]
    0x00, 0x48,

    // [Command PEC 15] | [0]
    0xB9, 0xD8

    // Precomputed PEC, shifted one bit to the left:
    // https://emn178.github.io/online-tools/crc/?input=0048&source=text&input_type=hex&output_type=hex_upper&model=custom&width=15&poly=C599&init=10&xorout=0&refin=0&refout=0
    // python -c "print(hex(0x5CEC << 1))"
};
/**
 * RDACD
 */
uint8_t const R_AvgCell_D[] = {
    // [00000] | [Command = 000 | 01001010]
    0x00, 0x4A,

    // [Command PEC 15] | [0]
    0x24, 0x8E

    // Precomputed PEC, shifted one bit to the left:
    // https://emn178.github.io/online-tools/crc/?input=004A&source=text&input_type=hex&output_type=hex_upper&model=custom&width=15&poly=C599&init=10&xorout=0&refin=0&refout=0
    // python -c "print(hex(0x1247 << 1))"
};
/**
 * RDACE
 */
uint8_t const R_AvgCell_E[] = {
    // [00000] | [Command = 000 | 01001001]
    0x00, 0x49,

    // [Command PEC 15] | [0]
    0x32, 0xEA

    // Precomputed PEC, shifted one bit to the left:
    // https://emn178.github.io/online-tools/crc/?input=0049&source=text&input_type=hex&output_type=hex_upper&model=custom&width=15&poly=C599&init=10&xorout=0&refin=0&refout=0
    // python -c "print(hex(0x1975 << 1))"
};
/**
 * RDACF
 */
uint8_t const R_AvgCell_F[] = {
    // [00000] | [Command = 000 | 01001011]
    0x00, 0x4B,

    // [Command PEC 15] | [0]
    0xAF, 0xBC

    // Precomputed PEC, shifted one bit to the left:
    // https://emn178.github.io/online-tools/crc/?input=004B&source=text&input_type=hex&output_type=hex_upper&model=custom&width=15&poly=C599&init=10&xorout=0&refin=0&refout=0
    // python -c "print(hex(0x57DE << 1))"
};

/**
 * Data Sheet pages (Rev. 0, from 1 to 83): 23
 *
 * Command: ADAX (Read GPIOs)
 * - Bit 10: 1
 * - Bit 09: 0
 * - Bit 08: OW = false // TODO see previous TODO about OW
 * - Bit 07: PUP = X (doesn't matter if OW=false, set to 0 in our case)
 * - Bit 06: CH[4] = 0 (read all GPIOs and other AUX inputs)
 * - Bit 05: 0
 * - Bit 04: 1
 * - Bit 03 to 00: CH[3..0] = 0 (read all GPIOs and other AUX inputs)
 */
uint8_t const ADAX[] = {
    // [00000] | [Command = 100 | 00010000]
    0x04, 0x10,

    // [Command PEC 15] | [0]
    0x51, 0x14

    // Precomputed PEC, shifted one bit to the left:
    // https://emn178.github.io/online-tools/crc/?input=0410&source=text&input_type=hex&output_type=hex_upper&model=custom&width=15&poly=C599&init=10&xorout=0&refin=0&refout=0
    // python -c "print(hex(0x288A << 1))"
};

/**
 * PLAUX
 */
uint8_t const PL_AUX[] = {
    // [00000] | [Command = 111 | 00011110]
    0x07, 0x1E,

    // [Command PEC 15] | [0]
    0x86, 0x34

    // Precomputed PEC, shifted one bit to the left:
    // https://emn178.github.io/online-tools/crc/?input=071E&source=text&input_type=hex&output_type=hex_upper&model=custom&width=15&poly=C599&init=10&xorout=0&refin=0&refout=0
    // python -c "print(hex(0x431A << 1))"
};
/**
 * RDAUXC
 */
uint8_t const R_AUX_C[] = {
    // [00000] | [Command = 000 | 00011011]
    0x00, 0x1B,

    // [Command PEC 15] | [0]
    0x13, 0x18

    // Precomputed PEC, shifted one bit to the left:
    // https://emn178.github.io/online-tools/crc/?input=001B&source=text&input_type=hex&output_type=hex_upper&model=custom&width=15&poly=C599&init=10&xorout=0&refin=0&refout=0
    // python -c "print(hex(0x098C << 1))"
};
