SIMD_HEADER=simd-support/simd-rvv256.h

include $(top_srcdir)/rdft/simd/codlist.mk
include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_RVV

noinst_LTLIBRARIES = librdft_rvv256_codelets.la
BUILT_SOURCES = $(EXTRA_DIST)
librdft_rvv256_codelets_la_SOURCES = $(BUILT_SOURCES)

endif
