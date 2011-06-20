AM_CFLAGS = $(SSE2_CFLAGS)
noinst_LTLIBRARIES = librdft_sse2_codelets.la
SIMD_HEADER=simd-sse2.h

include $(top_srcdir)/rdft/simd/codlist.mk
include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_SSE2
BUILT_SOURCES = $(EXTRA_DIST)
endif

librdft_sse2_codelets_la_SOURCES = $(BUILT_SOURCES)


