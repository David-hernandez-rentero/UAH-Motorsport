#ifndef G474RE_BMS_ERROR_H
#define G474RE_BMS_ERROR_H

#define VARARGS_HEAD(x, ...) x
#define VARARGS_OR_0(...) VARARGS_HEAD(__VA_OPT__(, ) 0) __VA_ARGS__

#define STRINGIFY_KEY(x) #x
#define STRINGIFY_VALUE(x) STRINGIFY_KEY(x)
#define CEIL_LOG2(N)                                                           \
  ((N) <= 1 /**/                                                               \
       ? 0                                                                     \
       : ((N) & ((N) - 1)) /**/                                                \
             ? (sizeof(unsigned long long) * 8 - __builtin_clzll(N))           \
             : (sizeof(unsigned long long) * 8 - __builtin_clzll((N) - 1)))

// https://stackoverflow.com/a/1644898
#define TRACE(fmt, ...)                                                        \
  do {                                                                         \
    printf("[%06lX] %27s:%-3d:%40s():\t\t" fmt, HAL_GetTick(), __FILE__,       \
           __LINE__, __func__, ##__VA_ARGS__);                                 \
  } while (0)

#define MUST_USE __attribute__((warn_unused_result))

#define ERROR_CRC_RX 0x01
#define ERROR_CMD_COUNTER 0x02
#define ERROR_SPI_TX 0x04
#define ERROR_SPI_RX 0x08
#define ERROR_ADBMS_PROTOCOL_VIOLATION 0x10
#define ERROR_CAN_TX 0x20

#define error MUST_USE int
// NOLINTBEGIN(bugprone-macro-parentheses)
#define IF_OK(err, x)                                                          \
  do {                                                                         \
    if (!(err))                                                                \
      x;                                                                       \
  } while (0)
#define BUBBLE_ERR(err, x) IF_OK(err, err |= (x))
// NOLINTEND(bugprone-macro-parentheses)

#endif // G474RE_BMS_ERROR_H
