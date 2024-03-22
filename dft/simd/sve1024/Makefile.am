AM_CFLAGS = $(SVE_CFLAGS)
SIMD_HEADER=simd-support/simd-maskedsve1024.h

include $(top_srcdir)/dft/simd/codlist.mk
include $(top_srcdir)/dft/simd/simd.mk

if HAVE_SVE

BUILT_SOURCES = $(EXTRA_DIST)
noinst_LTLIBRARIES = libdft_sve1024_codelets.la
libdft_sve1024_codelets_la_SOURCES = $(BUILT_SOURCES)

endif
