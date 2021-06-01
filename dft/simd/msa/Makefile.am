AM_CFLAGS = $(MSA_CFLAGS)
SIMD_HEADER=simd-support/simd-msa.h

include $(top_srcdir)/dft/simd/codlist.mk
include $(top_srcdir)/dft/simd/simd.mk

if HAVE_MSA

BUILT_SOURCES = $(EXTRA_DIST)
noinst_LTLIBRARIES = libdft_msa_codelets.la
libdft_msa_codelets_la_SOURCES = $(BUILT_SOURCES)

endif
