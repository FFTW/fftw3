AM_CFLAGS = $(ALTIVEC_CFLAGS)
noinst_LTLIBRARIES = libdft_altivec_codelets.la
SIMD_HEADER=simd-altivec.h

include $(top_srcdir)/dft/simd/codlist.mk
include $(top_srcdir)/dft/simd/simd.mk

if HAVE_ALTIVEC
BUILT_SOURCES = $(EXTRA_DIST)
endif

libdft_altivec_codelets_la_SOURCES = $(BUILT_SOURCES)


