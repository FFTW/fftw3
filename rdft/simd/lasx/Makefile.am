AM_CFLAGS = $(LASX_CFLAGS)

include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_LASX

noinst_LTLIBRARIES = librdft_lasx_codelets.la
librdft_lasx_codelets_la_SOURCES = $(SIMD_SOURCES)

endif
