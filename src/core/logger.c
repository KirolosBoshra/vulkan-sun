#include "logger.h"
#include "utils/globals.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static LogLevel minLevel = LOG_TRACE;

static const char *levelLabel(LogLevel level) {
  switch (level) {
  case LOG_TRACE:
    return "TRACE";
  case LOG_INFO:
    return "INFO";
  case LOG_WARN:
    return "WARN";
  case LOG_ERROR:
    return "ERROR";
  }
  return "?";
}

static const char *levelColor(LogLevel level) {
  switch (level) {
  case LOG_TRACE:
    return "\033[90m";
  case LOG_INFO:
    return "\033[37m";
  case LOG_WARN:
    return "\033[93m";
  case LOG_ERROR:
    return "\033[91m";
  }
  return "\033[0m";
}

void log_set_level(LogLevel level) { minLevel = level; }

void log_write(LogLevel level, const char *file, i32 line, const char *fmt,
                  ...) {
  if (level < minLevel)
    return;

  time_t t = time(NULL);
  struct tm tm;
  localtime_s(&tm, &t);

  char msg[4096];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(msg, sizeof msg, fmt, ap);
  va_end(ap);

  char timeBuf[16];
  strftime(timeBuf, sizeof timeBuf, "%H:%M:%S", &tm);

  fprintf(stderr, "%s[%s] [%s] %s:%d  %s\033[0m\n",
          levelColor(level), timeBuf, levelLabel(level), file, line, msg);
}
