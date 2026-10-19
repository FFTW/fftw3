AM_CFLAGS = $(KCVI_CFLAGS)

include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_KCVI

noinst_LTLIBRARIES = librdft_kcvi_codelets.la
librdft_kcvi_codelets_la_SOURCES = $(SIMD_SOURCES)

endif


