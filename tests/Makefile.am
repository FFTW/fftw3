AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/libbench2	\
-I$(top_srcdir)/dft -I$(top_srcdir)/rdft -I$(top_srcdir)/reodft	\
-I$(top_srcdir)/threads -I$(top_srcdir)/api 

noinst_PROGRAMS = bench
EXTRA_DIST = check.pl

if THREADS
LIBFFTWTHREADS = $(top_builddir)/threads/libfftw3@PREC_SUFFIX@_threads.la
else
LIBFFTWTHREADS = 
endif

bench_SOURCES = bench.c hook.c
bench_LDADD = $(LIBFFTWTHREADS)				\
$(top_builddir)/libfftw3@PREC_SUFFIX@.la		\
$(top_builddir)/libbench2/libbench2.a $(THREADLIBS)

check-local: bench
	perl -w $(srcdir)/check.pl -r -c=10 -v --nowisdom `pwd`/bench
if THREADS
	perl -w $(srcdir)/check.pl -r -c=10 -v --nthreads=2 --nowisdom `pwd`/bench
endif

bigcheck: bench
	perl -w $(srcdir)/check.pl -a -v --nowisdom `pwd`/bench
if THREADS
	perl -w $(srcdir)/check.pl -a -v --nthreads=2 --nowisdom `pwd`/bench
	perl -w $(srcdir)/check.pl -a -v --nthreads=3 --nowisdom `pwd`/bench
	perl -w $(srcdir)/check.pl -a -v --nthreads=10 --nowisdom `pwd`/bench
endif
