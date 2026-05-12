#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
    VS_LOG_TRACE,
    VS_LOG_INFO,
    VS_LOG_WARN,
    VS_LOG_ERROR,
} VsLogLevel;

#ifdef VS_LOGGING

#define vs_log_trace(fmt, ...)  vs_log_write(VS_LOG_TRACE,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define vs_log_info(fmt, ...)   vs_log_write(VS_LOG_INFO,   __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define vs_log_warn(fmt, ...)   vs_log_write(VS_LOG_WARN,   __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define vs_log_error(fmt, ...)  vs_log_write(VS_LOG_ERROR,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#else

#define vs_log_trace(fmt, ...)  ((void)0)
#define vs_log_info(fmt, ...)   ((void)0)
#define vs_log_warn(fmt, ...)   ((void)0)
#define vs_log_error(fmt, ...)  ((void)0)

#endif

void vs_log_write(VsLogLevel level, const char *file, int line, const char *fmt, ...);
void vs_log_set_level(VsLogLevel level);

#endif
