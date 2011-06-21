AM_CFLAGS = $(ALTIVEC_CFLAGS)
noinst_LTLIBRARIES = librdft_altivec_codelets.la
SIMD_HEADER=simd-altivec.h

include $(top_srcdir)/rdft/simd/codlist.mk
include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_ALTIVEC
BUILT_SOURCES = $(EXTRA_DIST)
endif

librdft_altivec_codelets_la_SOURCES = $(BUILT_SOURCES)


