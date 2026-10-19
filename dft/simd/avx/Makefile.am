AM_CFLAGS = $(AVX_CFLAGS)

include $(top_srcdir)/dft/simd/simd.mk

if HAVE_AVX

noinst_LTLIBRARIES = libdft_avx_codelets.la
libdft_avx_codelets_la_SOURCES = $(SIMD_SOURCES)

endif
