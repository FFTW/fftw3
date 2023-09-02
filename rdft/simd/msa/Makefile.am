AM_CFLAGS = $(MSA_CFLAGS)
SIMD_HEADER=simd-support/simd-msa.h

include $(top_srcdir)/rdft/simd/codlist.mk
include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_MSA

noinst_LTLIBRARIES = librdft_msa_codelets.la
BUILT_SOURCES = $(EXTRA_DIST)
librdft_msa_codelets_la_SOURCES = $(BUILT_SOURCES)

endif
