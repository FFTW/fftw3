AM_CFLAGS = $(AVX2_CFLAGS)

include $(top_srcdir)/dft/simd/simd.mk

if HAVE_AVX2

noinst_LTLIBRARIES = libdft_avx2_128_codelets.la
libdft_avx2_128_codelets_la_SOURCES = $(SIMD_SOURCES)

endif
