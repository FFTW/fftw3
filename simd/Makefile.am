AM_CPPFLAGS = -I$(top_srcdir)/kernel
noinst_LTLIBRARIES = libsimd.la libsimd_safe.la

libsimd_la_CFLAGS = $(AM_CFLAGS) $(SIMD_CFLAGS)
libsimd_la_SOURCES = 3dnow.c altivec.c sse-aux.c sse.c sse2-aux.c	\
sse2.c simd-3dnow.h simd-altivec.h simd-sse.h simd-sse2.h simd.h
libsimd_la_LIBADD = libsimd_safe.la

# Don't use $(SIMD_CFLAGS) with routines that can be called on non-SIMD
# machines.  In particular, fftw_taint contains unportable code if
# compiled with -msse.
libsimd_safe_la_SOURCES = taint.c
