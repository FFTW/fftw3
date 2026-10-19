AM_CFLAGS = $(LASX_CFLAGS)

include $(top_srcdir)/dft/simd/simd.mk

if HAVE_LASX

noinst_LTLIBRARIES = libdft_lasx_codelets.la
libdft_lasx_codelets_la_SOURCES = $(SIMD_SOURCES)

endif
