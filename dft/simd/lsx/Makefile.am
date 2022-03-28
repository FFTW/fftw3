AM_CFLAGS = $(LSX_CFLAGS)
SIMD_HEADER=simd-support/simd-lsx.h

include $(top_srcdir)/dft/simd/codlist.mk
include $(top_srcdir)/dft/simd/simd.mk

if HAVE_LSX

BUILT_SOURCES = $(EXTRA_DIST)
noinst_LTLIBRARIES = libdft_lsx_codelets.la
libdft_lsx_codelets_la_SOURCES = $(BUILT_SOURCES)

endif
