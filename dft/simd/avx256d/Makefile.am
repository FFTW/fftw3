AM_CFLAGS = $(AVX_CFLAGS)
noinst_LTLIBRARIES = libdft_avx256d_codelets.la
SIMD_HEADER=simd-avx256d.h

include $(top_srcdir)/dft/simd/codlist.mk
include $(top_srcdir)/dft/simd/simd.mk

if HAVE_AVX256D
BUILT_SOURCES = $(EXTRA_DIST)
endif

libdft_avx256d_codelets_la_SOURCES = $(BUILT_SOURCES)
