#ifndef PEC_10_CRC
#define PEC_10_CRC

#include <stddef.h>
#include <stdint.h>

uint_fast16_t pec_10_crc(uint8_t const *data, size_t byte_len,
                         size_t extra_bit_len);

#endif /* PEC_10_CRC */
