#include "fftw-spu.h"

/* simple-minded memory allocator */

static char mem[160*1024] __attribute__ ((aligned (ALIGNMENT)));
static char *allocptr;

void X(spu_alloc_reset)(void)
{
     allocptr = mem;
}

void *X(spu_alloc)(size_t sz)
{
     void *p = allocptr;
     sz = (sz + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);       /* align SZ up */
     allocptr = p + sz;
     return p;
}

size_t X(spu_alloc_avail)(void)
{
     return (mem + sizeof(mem)) - allocptr;
}
