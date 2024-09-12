#include "log.h"
#include <stdio.h>
#include <stdarg.h>

void mcugdx_log(const char *TAG, const char *format, ...) {
    va_list args;
    va_start(args, format);
    printf("[%s] ", TAG);
    vprintf(format, args);
    va_end(args);
}

void mcugdx_loge(const char *TAG, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "[ERROR] [%s] ", TAG);
    vfprintf(stderr, format, args);
    va_end(args);
}
