AM_CFLAGS = $(ALTIVEC_CFLAGS)

include $(top_srcdir)/dft/simd/simd.mk

if HAVE_ALTIVEC

noinst_LTLIBRARIES = libdft_altivec_codelets.la
libdft_altivec_codelets_la_SOURCES = $(SIMD_SOURCES)

endif
