AM_CFLAGS = $(AVX2_CFLAGS)

include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_AVX2

noinst_LTLIBRARIES = librdft_avx2_128_codelets.la
librdft_avx2_128_codelets_la_SOURCES = $(SIMD_SOURCES)

endif


