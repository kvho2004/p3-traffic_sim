
#ifndef LOGGER_H
#define LOGGER_H

void log_event(const char* source, const char* msg);
void log_fmt  (const char* source, const char* fmt, ...);

#endif