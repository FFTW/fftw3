AM_CFLAGS = $(KCVI_CFLAGS)

include $(top_srcdir)/dft/simd/simd.mk

if HAVE_KCVI

noinst_LTLIBRARIES = libdft_kcvi_codelets.la
libdft_kcvi_codelets_la_SOURCES = $(SIMD_SOURCES)

endif
