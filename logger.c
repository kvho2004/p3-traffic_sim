#define _POSIX_C_SOURCE 200809L
#include "logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <time.h>

static pthread_mutex_t log_mtx = PTHREAD_MUTEX_INITIALIZER;

void log_event(const char* source, const char* msg) {
    pthread_mutex_lock(&log_mtx);
    printf("[%-12s] %s\n", source, msg);
    fflush(stdout);
    pthread_mutex_unlock(&log_mtx);
}

void log_fmt(const char* source, const char* fmt, ...) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long ms = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000); // convert to milliseconds
    va_list args;
    pthread_mutex_lock(&log_mtx);
    printf("[%7ld ms][%-12s] ", ms, source);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf(" [%ld ms]\n", ms);
    fflush(stdout);
    pthread_mutex_unlock(&log_mtx);
}