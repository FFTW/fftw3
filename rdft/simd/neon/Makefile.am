AM_CFLAGS = $(NEON_CFLAGS)

include $(top_srcdir)/rdft/simd/simd.mk

if HAVE_NEON

noinst_LTLIBRARIES = librdft_neon_codelets.la
librdft_neon_codelets_la_SOURCES = $(SIMD_SOURCES)

endif
