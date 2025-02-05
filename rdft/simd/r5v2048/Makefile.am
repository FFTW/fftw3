SIMD_HEADER=simd-support/simd-r5v2048.h

include $(top_srcdir)/rdft/simd/codlist.mk
include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_R5V

noinst_LTLIBRARIES = librdft_r5v2048_codelets.la
BUILT_SOURCES = $(EXTRA_DIST)
librdft_r5v2048_codelets_la_SOURCES = $(BUILT_SOURCES)

endif


