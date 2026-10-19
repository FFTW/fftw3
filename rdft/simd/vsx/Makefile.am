AM_CFLAGS = $(VSX_CFLAGS)

include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_VSX

noinst_LTLIBRARIES = librdft_vsx_codelets.la
librdft_vsx_codelets_la_SOURCES = $(SIMD_SOURCES)

endif


