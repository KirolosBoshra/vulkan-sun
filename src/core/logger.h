#ifndef LOGGER_H
#define LOGGER_H

#include "utils/globals.h"
typedef enum {
  LOG_TRACE,
  LOG_INFO,
  LOG_WARN,
  LOG_ERROR,
} LogLevel;

#ifdef LOGGING

#define log_trace(fmt, ...)                                                 \
  log_write(LOG_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)                                                  \
  log_write(LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...)                                                  \
  log_write(LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...)                                                 \
  log_write(LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#else

#define log_trace(fmt, ...) ((void)0)
#define log_info(fmt, ...) ((void)0)
#define log_warn(fmt, ...) ((void)0)
#define log_error(fmt, ...) ((void)0)

#endif

void log_write(LogLevel level, const char *file, i32 line, const char *fmt,
                  ...);
void log_set_level(LogLevel level);

#endif
