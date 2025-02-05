AM_CFLAGS = $(LSX_CFLAGS)
SIMD_HEADER=simd-support/simd-lsx.h

include $(top_srcdir)/rdft/simd/codlist.mk
include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_LSX

BUILT_SOURCES = $(EXTRA_DIST)
noinst_LTLIBRARIES = librdft_lsx_codelets.la
librdft_lsx_codelets_la_SOURCES = $(BUILT_SOURCES)

endif
