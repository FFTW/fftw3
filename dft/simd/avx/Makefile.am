AM_CFLAGS = $(AVX_CFLAGS)
noinst_LTLIBRARIES = libdft_avx_codelets.la
SIMD_HEADER=simd-avx.h

include $(top_srcdir)/dft/simd/codlist.mk
include $(top_srcdir)/dft/simd/simd.mk

if HAVE_AVX
BUILT_SOURCES = $(EXTRA_DIST)
endif

libdft_avx_codelets_la_SOURCES = $(BUILT_SOURCES)
