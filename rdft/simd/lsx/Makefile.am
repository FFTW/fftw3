AM_CFLAGS = $(LSX_CFLAGS)

include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_LSX

noinst_LTLIBRARIES = librdft_lsx_codelets.la
librdft_lsx_codelets_la_SOURCES = $(SIMD_SOURCES)

endif
