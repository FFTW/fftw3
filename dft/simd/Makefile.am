AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft -I$(top_srcdir)/simd
noinst_LTLIBRARIES = libdft_simd.la
SUBDIRS = codelets
CFLAGS = @CFLAGS@ $(SIMD_CFLAGS)
libdft_simd_la_SOURCES = n1b.c n1f.c n2b.c n2f.c q1b.c q1f.c t1b.c	\
t1f.c n1b.h n1f.h n2b.h n2f.h q1b.h q1f.h t1b.h t1f.h
