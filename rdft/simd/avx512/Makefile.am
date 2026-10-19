AM_CFLAGS = $(AVX512_CFLAGS)

include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_AVX512

noinst_LTLIBRARIES = librdft_avx512_codelets.la
librdft_avx512_codelets_la_SOURCES = $(SIMD_SOURCES)

endif


