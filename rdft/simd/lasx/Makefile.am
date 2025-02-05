AM_CFLAGS = $(LASX_CFLAGS)
SIMD_HEADER=simd-support/simd-lasx.h

include $(top_srcdir)/rdft/simd/codlist.mk
include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_LASX

BUILT_SOURCES = $(EXTRA_DIST)
noinst_LTLIBRARIES = librdft_lasx_codelets.la
librdft_lasx_codelets_la_SOURCES = $(BUILT_SOURCES)

endif
