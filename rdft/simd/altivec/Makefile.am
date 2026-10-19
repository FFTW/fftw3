AM_CFLAGS = $(ALTIVEC_CFLAGS)

include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_ALTIVEC

noinst_LTLIBRARIES = librdft_altivec_codelets.la
librdft_altivec_codelets_la_SOURCES = $(SIMD_SOURCES)

endif
