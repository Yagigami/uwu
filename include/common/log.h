#ifndef C_UWU_COMMON_LOG_H
#define C_UWU_COMMON_LOG_H

#include <stream/stream.h>

#include <stdarg.h>

long uwuprintf(const char *fmt, ...);
long uwufprintf(Stream stream, const char *fmt, ...);
long uwuvfprintf(Stream stream, const char *fmt, va_list args);
long uwusprintf(char *buf, const char *fmt, ...);
long uwuvsprintf(char *buf, const char *fmt, va_list args);
long uwuvsnprintf(char *buf, long n, const char *fmt, va_list args);

#endif /* C_UWU_COMMON_LOG_H */

