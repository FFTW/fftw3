AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/libbench	\
-I$(top_srcdir)/dft -I$(top_srcdir)/rdft -I$(top_srcdir)/reodft	\
-I$(top_srcdir)/threads

noinst_PROGRAMS = bench

bench_SOURCES = bench.c
bench_LDADD = $(top_builddir)/libfftw3@PREC_SUFFIX@.la	\
$(top_builddir)/libbench/libbench.a $(THREADLIBS)


