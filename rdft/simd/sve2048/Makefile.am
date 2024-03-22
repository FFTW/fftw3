AM_CFLAGS = $(SVE_CFLAGS)
SIMD_HEADER=simd-support/simd-maskedsve2048.h

include $(top_srcdir)/rdft/simd/codlist.mk
include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_SVE

noinst_LTLIBRARIES = librdft_sve2048_codelets.la
BUILT_SOURCES = $(EXTRA_DIST)
librdft_sve2048_codelets_la_SOURCES = $(BUILT_SOURCES)

endif
