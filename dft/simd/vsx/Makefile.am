AM_CFLAGS = $(VSX_CFLAGS)

include $(top_srcdir)/dft/simd/simd.mk

if HAVE_VSX

noinst_LTLIBRARIES = libdft_vsx_codelets.la
libdft_vsx_codelets_la_SOURCES = $(SIMD_SOURCES)

endif
