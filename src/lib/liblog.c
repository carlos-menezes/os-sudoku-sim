#include "liblog.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static const char *levels[] = {
    "INFO", "ERROR", "FATAL"
};

static void stdout_callback(struct log_event* e) {
    char buf[16];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", e->time)] = '\0';
    printf("%s %s: ", buf, levels[e->level]);
    vprintf(e->fmt, e->args);
    printf("\n");
}

static void file_callback(struct log_event* e) {
    char buf[16];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", e->time)] = '\0';
    fprintf(e->file, "%s %s: ", buf, levels[e->level]);
    vfprintf(e->file, e->fmt, e->args);
    fputs("\n", e->file);
    fflush(e->file);
}

const char* log_level_str(int level) {
    return levels[level];
}

void log_info(FILE *file, const char *fmt, ...) {
    struct log_event e = {
        .fmt = fmt,
        .file = file,
        .level = LOG_INFO
    };

    time_t t = time(NULL);
    e.time = localtime(&t);

    va_start(e.args, fmt);
    if (file != NULL) {
        file_callback(&e);
    }
    va_end(e.args);
    va_start(e.args, fmt);
    stdout_callback(&e);
    va_end(e.args);
}

void log_error(FILE *file, const char *fmt, ...) {
    struct log_event e = {
        .fmt = fmt,
        .file = file,
        .level = LOG_ERROR
    };

    time_t t = time(NULL);
    e.time = localtime(&t);

    va_start(e.args, fmt);
    if (file != NULL) {
        file_callback(&e);
    }
    va_end(e.args);
    va_start(e.args, fmt);
    stdout_callback(&e);
    va_end(e.args);
}

void log_fatal(FILE *file, const char *fmt, ...) {
    struct log_event e = {
        .fmt = fmt,
        .file = file,
        .level = LOG_ERROR
    };

    time_t t = time(NULL);
    e.time = localtime(&t);

    va_start(e.args, fmt);
    if (file != NULL) {
        file_callback(&e);
    }
    va_end(e.args);
    va_start(e.args, fmt);
    stdout_callback(&e);
    va_end(e.args);
}