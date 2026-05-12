#include "logger.h"
#include "utils/globals.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static VsLogLevel minLevel = VS_LOG_TRACE;

static const char *levelLabel(VsLogLevel level) {
  switch (level) {
  case VS_LOG_TRACE:
    return "TRACE";
  case VS_LOG_INFO:
    return "INFO";
  case VS_LOG_WARN:
    return "WARN";
  case VS_LOG_ERROR:
    return "ERROR";
  }
  return "?";
}

static const char *levelColor(VsLogLevel level) {
  switch (level) {
  case VS_LOG_TRACE:
    return "\033[90m";
  case VS_LOG_INFO:
    return "\033[37m";
  case VS_LOG_WARN:
    return "\033[93m";
  case VS_LOG_ERROR:
    return "\033[91m";
  }
  return "\033[0m";
}

void vs_log_set_level(VsLogLevel level) { minLevel = level; }

void vs_log_write(VsLogLevel level, const char *file, i32 line, const char *fmt,
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

  fprintf(stderr, "%s[%s] %s:%d  %s\033[0m\n", levelColor(level),
          levelLabel(level), file, line, msg);
}
