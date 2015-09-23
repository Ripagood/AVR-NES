#include  <stdio.h>
#include  <stdlib.h>
#include  <assert.h>
#include  <stdarg.h>
#include  "globals.h"
#define LOG_LINE_LENGTH 1024
void debug_printf (const char *format, ... ) {
#ifdef LOG
  va_list arg;
  va_start (arg, format);
  char logBuffer[LOG_LINE_LENGTH];
  vsnprintf(logBuffer,LOG_LINE_LENGTH-1,format,arg);
  fprintf(stderr,logBuffer,LOG_LINE_LENGTH);
  va_end (arg);
#endif
}