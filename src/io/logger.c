#include "io/logger.h"
#include <string.h>
#include <stdarg.h>
#include <time.h>

void logger_init(Logger *l, const char *filename, LogLevel min_level) {
    l->file = NULL;
    l->min_level = min_level;
    l->color = false;
    l->timestamp = true;

    strncpy(l->filename, filename, sizeof(l->filename) - 1);

    if (filename && filename[0] != '\0') {
        l->file = fopen(filename, "a");
    }
}

void logger_destroy(Logger *l) {
    if (l->file) {
        fclose(l->file);
        l->file = NULL;
    }
}

const char *logger_level_name(LogLevel level) {
    switch (level) {
        case LOG_TRACE: return "TRACE";
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO:  return "INFO";
        case LOG_WARN:  return "WARN";
        case LOG_ERROR: return "ERROR";
        case LOG_FATAL: return "FATAL";
        default:        return "UNKNOWN";
    }
}

void logger_log(Logger *l, LogLevel level, const char *file, int line, const char *fmt, ...) {
    if (level < l->min_level) return;

    FILE *out = l->file ? l->file : stderr;

    if (l->timestamp) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char tbuf[32];
        strftime(tbuf, sizeof(tbuf), "%H:%M:%S", tm_info);
        fprintf(out, "%s ", tbuf);
    }

    fprintf(out, "[%s] ", logger_level_name(level));

    if (l->color) {
        if (level >= LOG_ERROR) fprintf(out, "\033[31m");
        else if (level >= LOG_WARN) fprintf(out, "\033[33m");
        else if (level <= LOG_DEBUG) fprintf(out, "\033[90m");
    }

    fprintf(out, "%s:%d: ", file, line);

    va_list args;
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    va_end(args);

    if (l->color) fprintf(out, "\033[0m");
    fprintf(out, "\n");
    fflush(out);
}

void logger_set_level(Logger *l, LogLevel level) {
    l->min_level = level;
}
