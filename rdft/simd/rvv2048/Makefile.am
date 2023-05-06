SIMD_HEADER=simd-support/simd-rvv2048.h

include $(top_srcdir)/rdft/simd/codlist.mk
include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_RVV

noinst_LTLIBRARIES = librdft_rvv2048_codelets.la
BUILT_SOURCES = $(EXTRA_DIST)
librdft_rvv2048_codelets_la_SOURCES = $(BUILT_SOURCES)

endif
