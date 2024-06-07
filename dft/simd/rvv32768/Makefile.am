SIMD_HEADER=simd-support/simd-rvv32768.h

include $(top_srcdir)/dft/simd/codlist.mk
include $(top_srcdir)/dft/simd/simd.mk

if HAVE_RVV

BUILT_SOURCES = $(EXTRA_DIST)
noinst_LTLIBRARIES = libdft_rvv32768_codelets.la
libdft_rvv32768_codelets_la_SOURCES = $(BUILT_SOURCES)

endif
