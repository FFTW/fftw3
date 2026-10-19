AM_CFLAGS = $(AVX_128_FMA_CFLAGS)

include $(top_srcdir)/dft/simd/simd.mk

if HAVE_AVX_128_FMA

noinst_LTLIBRARIES = libdft_avx_128_fma_codelets.la
libdft_avx_128_fma_codelets_la_SOURCES = $(SIMD_SOURCES)

endif
