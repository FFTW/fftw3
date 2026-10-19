AM_CFLAGS = $(SVE_CFLAGS)

include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_SVE

noinst_LTLIBRARIES = librdft_sve1024_codelets.la
librdft_sve1024_codelets_la_SOURCES = $(SIMD_SOURCES)

endif
