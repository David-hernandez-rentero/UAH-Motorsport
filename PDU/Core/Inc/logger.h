//
// Created by guille on 8/3/26.
//

#ifndef READY_TO_DRIVE_LOGGER_H
#define READY_TO_DRIVE_LOGGER_H

// ===========================================================================
// LOGGER — wrapper sobre printf con timestamp y nivel
//
// Uso:
//   LOG_INFO("texto %d", valor);
//   LOG_WARN("aviso");
//   LOG_ERROR("fallo critico %lu", tick);
//   LOG_DEBUG("debug");
//
// Para filtrar nivel mínimo visible:
//   log_min_level = LOG_LEVEL_WARN;  // solo WARN y ERROR
// ===========================================================================

typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
} LogLevel_t;

// Nivel mínimo visible por terminal (LOG_LEVEL_DEBUG muestra todo)
extern volatile LogLevel_t log_min_level;

void log_write(LogLevel_t level, const char *fmt, ...);

#define LOG_DEBUG(fmt, ...)  log_write(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)   log_write(LOG_LEVEL_INFO,  fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)   log_write(LOG_LEVEL_WARN,  fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)  log_write(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)

#endif //READY_TO_DRIVE_LOGGER_H