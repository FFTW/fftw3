AM_CFLAGS = $(LSX_CFLAGS)

include $(top_srcdir)/dft/simd/simd.mk

if HAVE_LSX

noinst_LTLIBRARIES = libdft_lsx_codelets.la
libdft_lsx_codelets_la_SOURCES = $(SIMD_SOURCES)

endif
