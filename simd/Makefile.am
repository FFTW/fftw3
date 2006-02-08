SUBDIRS=nonportable
AM_CPPFLAGS = -I$(top_srcdir)/kernel
noinst_LTLIBRARIES = libsimd.la

libsimd_la_SOURCES = altivec.c sse.c sse2.c simd-altivec.h simd-sse.h	\
simd-sse2.h simd.h taint.c x86-cpuid.h
