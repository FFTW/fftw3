AM_CPPFLAGS = -I$(top_srcdir)/kernel
noinst_LTLIBRARIES = libsimd_support.la libsimd_sse2_nonportable.la

libsimd_support_la_SOURCES = taint.c simd-common.h \
x86-cpuid.h amd64-cpuid.h \
simd-sse2.h sse2.c \
avx.c simd-avx.h \
avx-128-fma.c simd-avx-128-fma.h \
avx2.c simd-avx2.h simd-avx2-128.h \
avx512.c simd-avx512.h \
kcvi.c simd-kcvi.h \
altivec.c simd-altivec.h vsx.c simd-vsx.h \
neon.c simd-neon.h \
simd-generic128.h simd-generic256.h

# sse2-nonportable.c needs SSE2_CFLAGS, but Automake does not support
# per-object CFLAGS.  Thus we build a separate library.
libsimd_sse2_nonportable_la_CFLAGS = $(SSE2_CFLAGS)
libsimd_sse2_nonportable_la_SOURCES = sse2-nonportable.c 

