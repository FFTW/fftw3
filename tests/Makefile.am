AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/libbench	\
-I$(top_srcdir)/dft -I$(top_srcdir)/rdft
noinst_PROGRAMS = bench

bench_SOURCES = bench.c
bench_LDADD = $(top_builddir)/lib@PRECISION@fftw3.la	\
$(top_builddir)/libbench/libbench.la


