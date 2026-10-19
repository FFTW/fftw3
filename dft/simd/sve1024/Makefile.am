AM_CFLAGS = $(SVE_CFLAGS)

include $(top_srcdir)/dft/simd/simd.mk

if HAVE_SVE

noinst_LTLIBRARIES = libdft_sve1024_codelets.la
libdft_sve1024_codelets_la_SOURCES = $(SIMD_SOURCES)

endif
