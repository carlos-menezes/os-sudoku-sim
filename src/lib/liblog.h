#ifndef LIBLOG_H
#define LIBLOG_H

#include <stdio.h>

typedef struct {
    va_list args;
    struct tm *time;
    int level;
    FILE* file;
    const char* fmt;
} log_event;

enum { LOG_INFO, LOG_ERROR, LOG_FATAL };

void log_log(int level, FILE* file, const char* fmt, ...);

void log_info(FILE* file, const char* fmt, ...);
void log_error(FILE* file, const char* fmt, ...);
void log_fatal(FILE* file, const char* fmt, ...);

#endif