AM_CPPFLAGS = -I$(top_srcdir)/libbench2 -I$(top_srcdir)/api 

bin_SCRIPTS = fftw-wisdom-to-conf
bin_PROGRAMS = fftw@PREC_SUFFIX@-wisdom

BUILT_SOURCES = fftw-wisdom-to-conf
EXTRA_DIST = fftw-wisdom-to-conf.in

if THREADS
LIBFFTWTHREADS = $(top_builddir)/threads/libfftw3@PREC_SUFFIX@_threads.la
else
LIBFFTWTHREADS = 
endif

fftw@PREC_SUFFIX@_wisdom_SOURCES = fftw-wisdom.c
fftw@PREC_SUFFIX@_wisdom_LDADD = $(top_builddir)/tests/bench.o	\
$(LIBFFTWTHREADS) $(top_builddir)/libfftw3@PREC_SUFFIX@.la	\
$(top_builddir)/libbench2/libbench2.a $(THREADLIBS)
