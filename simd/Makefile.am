AM_CPPFLAGS = -I$(top_srcdir)/kernel
noinst_LTLIBRARIES = libsimd.la
CFLAGS = @CFLAGS@ $(SIMD_CFLAGS)
libsimd_la_SOURCES = 3dnow.c altivec.c sse-aux.c sse.c sse2-aux.c	\
sse2.c simd-3dnow.h simd-altivec.h simd-sse.h simd-sse2.h simd.h taint.c
