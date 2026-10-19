AM_CFLAGS = $(SSE2_CFLAGS)

include $(top_srcdir)/dft/simd/simd.mk

if HAVE_SSE2

noinst_LTLIBRARIES = libdft_sse2_codelets.la
libdft_sse2_codelets_la_SOURCES = $(SIMD_SOURCES)

endif
