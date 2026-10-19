AM_CFLAGS = $(SSE2_CFLAGS)

include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_SSE2

noinst_LTLIBRARIES = librdft_sse2_codelets.la
librdft_sse2_codelets_la_SOURCES = $(SIMD_SOURCES)

endif


