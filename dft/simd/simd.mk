include $(top_srcdir)/support/codelets.mk

AM_CPPFLAGS = -I $(top_srcdir)
SIMD_SOURCES = $(DFT_SIMD_CODELETS) genus.c codlist.c
EXTRA_DIST = $(SIMD_SOURCES)
