SIMD_HEADER=simd-support/simd-r5v8192.h

include $(top_srcdir)/dft/simd/codlist.mk
include $(top_srcdir)/dft/simd/simd.mk

if HAVE_R5V

BUILT_SOURCES = $(EXTRA_DIST)
noinst_LTLIBRARIES = libdft_r5v8192_codelets.la
libdft_r5v8192_codelets_la_SOURCES = $(BUILT_SOURCES)

endif
