AM_CFLAGS = $(AVX_CFLAGS)

include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_AVX

noinst_LTLIBRARIES = librdft_avx_codelets.la
librdft_avx_codelets_la_SOURCES = $(SIMD_SOURCES)

endif


