#include "logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>

static pthread_mutex_t log_mtx = PTHREAD_MUTEX_INITIALIZER;

void log_event(const char* source, const char* msg) {
    pthread_mutex_lock(&log_mtx);
    printf("[%-12s] %s\n", source, msg);
    fflush(stdout);
    pthread_mutex_unlock(&log_mtx);
}

void log_fmt(const char* source, const char* fmt, ...) {
    va_list args;
    pthread_mutex_lock(&log_mtx);
    printf("[%-12s] ", source);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    fflush(stdout);
    pthread_mutex_unlock(&log_mtx);
}