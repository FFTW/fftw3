#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "bench.h"

void ovtpvt(const char *format, ...)
{
     va_list ap;
     
     va_start(ap, format);
     vfprintf(stdout, format, ap);
     va_end(ap);
     fflush(stdout);
}
