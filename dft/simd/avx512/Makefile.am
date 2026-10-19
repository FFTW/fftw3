AM_CFLAGS = $(AVX512_CFLAGS)

include $(top_srcdir)/dft/simd/simd.mk

if HAVE_AVX512

noinst_LTLIBRARIES = libdft_avx512_codelets.la
libdft_avx512_codelets_la_SOURCES = $(SIMD_SOURCES)

endif
