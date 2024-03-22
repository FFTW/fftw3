SIMD_HEADER=simd-support/simd-rvv65536.h

include $(top_srcdir)/rdft/simd/codlist.mk
include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_RVV

noinst_LTLIBRARIES = librdft_rvv65536_codelets.la
BUILT_SOURCES = $(EXTRA_DIST)
librdft_rvv65536_codelets_la_SOURCES = $(BUILT_SOURCES)

endif
