AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/libbench2	\
-I$(top_srcdir)/dft -I$(top_srcdir)/rdft -I$(top_srcdir)/reodft	\
-I$(top_srcdir)/threads -I$(top_srcdir)/api 

noinst_PROGRAMS = bench

if THREADS
LIBFFTWTHREADS = $(top_builddir)/threads/libfftw3@PREC_SUFFIX@_threads.la
else
LIBFFTWTHREADS = 
endif

bench_SOURCES = bench.c
bench_LDADD = $(LIBFFTWTHREADS)				\
$(top_builddir)/libfftw3@PREC_SUFFIX@.la		\
$(top_builddir)/libbench2/libbench2.a $(THREADLIBS)




