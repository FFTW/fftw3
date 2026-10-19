AM_CFLAGS = $(NEON_CFLAGS)

include $(top_srcdir)/dft/simd/simd.mk

if HAVE_NEON

noinst_LTLIBRARIES = libdft_neon_codelets.la
libdft_neon_codelets_la_SOURCES = $(SIMD_SOURCES)

endif
