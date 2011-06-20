AM_CFLAGS = $(AVX_CFLAGS)
noinst_LTLIBRARIES = librdft_avx256d_codelets.la
SIMD_HEADER=simd-avx256d.h

include $(top_srcdir)/rdft/simd/codlist.mk
include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_AVX256D
BUILT_SOURCES = $(EXTRA_DIST)
endif

librdft_avx256d_codelets_la_SOURCES = $(BUILT_SOURCES)


