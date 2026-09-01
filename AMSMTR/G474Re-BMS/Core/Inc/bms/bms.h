#ifndef G474RE_BMS_BMS_H
#define G474RE_BMS_BMS_H

#include "macros.h"

#include <stdbool.h>

void bms_setup(void);
void bms_loop(void);
MUST_USE bool bms_is_overcurrent(void);

#endif // G474RE_BMS_BMS_H
