#include "log.h"
#include <cstdarg>

void log_write(const char* level, const char* fmt, ...)
{
    FILE* f = fopen("/tmp/aion_vr_layer.log", "a");
    if (!f) return;

    fprintf(f, "[%s] ", level);

    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);

    fprintf(f, "\n");
    fclose(f);
}
