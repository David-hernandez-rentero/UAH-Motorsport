#include "main.h"

#pragma GCC diagnostic push
// NOLINTNEXTLINE (clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wbuiltin-declaration-mismatch"

#define UBSAN_HANDLER(name)                                                    \
  void __ubsan_handle_##name(void *, ...) { Error_Handler(); }

UBSAN_HANDLER(add_overflow)
UBSAN_HANDLER(sub_overflow)
UBSAN_HANDLER(mul_overflow)
UBSAN_HANDLER(divrem_overflow)
UBSAN_HANDLER(negate_overflow)
UBSAN_HANDLER(pointer_overflow)
UBSAN_HANDLER(shift_out_of_bounds)
UBSAN_HANDLER(out_of_bounds)
UBSAN_HANDLER(load_invalid_value)
UBSAN_HANDLER(missing_return)
UBSAN_HANDLER(type_mismatch_v1)
UBSAN_HANDLER(function_type_mismatch)
UBSAN_HANDLER(builtin_unreachable)
UBSAN_HANDLER(nonnull_return_v1)
UBSAN_HANDLER(nonnull_arg)
UBSAN_HANDLER(vla_bound_not_positive)
UBSAN_HANDLER(invalid_builtin)

#pragma GCC diagnostic pop
