#pragma once
#include <cstdio>

#define LOG_INFO(fmt, ...)  log_write("INFO", fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) log_write("ERROR", fmt, ##__VA_ARGS__)

void log_write(const char* level, const char* fmt, ...);
