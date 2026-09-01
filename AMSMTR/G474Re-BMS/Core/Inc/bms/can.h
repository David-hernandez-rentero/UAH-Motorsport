#ifndef G474RE_BMS_CAN_H
#define G474RE_BMS_CAN_H

#include "macros.h"

#include <stddef.h>
#include <stdint.h>

error can_setup(void);
error can_send(uint32_t id, uint16_t data);

#endif // G474RE_BMS_CAN_H
