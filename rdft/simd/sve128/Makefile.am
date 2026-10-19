AM_CFLAGS = $(SVE_CFLAGS)

include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_SVE

noinst_LTLIBRARIES = librdft_sve128_codelets.la
librdft_sve128_codelets_la_SOURCES = $(SIMD_SOURCES)

endif
