AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/libbench	\
-I$(top_srcdir)/dft -I$(top_srcdir)/rdft -I$(top_srcdir)/reodft	\
-I$(top_srcdir)/threads

noinst_PROGRAMS = bench
noinst_LIBRARIES=libdebug.a

bench_SOURCES = bench.c
bench_LDADD = $(top_builddir)/libfftw3@PREC_SUFFIX@.la libdebug.a	\
$(top_builddir)/libbench/libbench.a $(THREADLIBS)

libdebug_a_SOURCES = dotens.c dotens2.c verify-dft.c verify-lib.c	\
verify-rdft.c verify-reodft.c debug.h verify.h



