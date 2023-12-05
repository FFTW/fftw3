AM_CPPFLAGS = -I $(top_srcdir)
AM_CFLAGS = $(STACK_ALIGN_CFLAGS)

if OPENMP
FFTWOMPLIB = libfftw3@PREC_SUFFIX@_omp.la
else
FFTWOMPLIB = 
endif

if HPX
FFTWHPXLIB = libfftw3@PREC_SUFFIX@_hpx.la
else
FFTWHPXLIB =
endif

if THREADS
if COMBINED_THREADS
noinst_LTLIBRARIES = libfftw3@PREC_SUFFIX@_threads.la
endif
if OPENMP
lib_LTLIBRARIES = libfftw3@PREC_SUFFIX@_threads.la $(FFTWOMPLIB)
endif
if HPX
lib_LTLIBRARIES = libfftw3@PREC_SUFFIX@_threads.la $(FFTWHPXLIB)
endif
endif

libfftw3@PREC_SUFFIX@_threads_la_SOURCES = api.c conf.c threads.c	\
threads.h dft-vrank-geq1.c ct.c rdft-vrank-geq1.c hc2hc.c		\
vrank-geq1-rdft2.c f77api.c f77funcs.h
libfftw3@PREC_SUFFIX@_threads_la_CFLAGS = $(AM_CFLAGS) $(PTHREAD_CFLAGS)
libfftw3@PREC_SUFFIX@_threads_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@
if !COMBINED_THREADS
libfftw3@PREC_SUFFIX@_threads_la_LIBADD = ../libfftw3@PREC_SUFFIX@.la
endif

if OPENMP
libfftw3@PREC_SUFFIX@_omp_la_SOURCES = api.c conf.c openmp.c	\
threads.h dft-vrank-geq1.c ct.c rdft-vrank-geq1.c hc2hc.c	\
vrank-geq1-rdft2.c f77api.c f77funcs.h
libfftw3@PREC_SUFFIX@_omp_la_CFLAGS = $(AM_CFLAGS) $(OPENMP_CFLAGS)
libfftw3@PREC_SUFFIX@_omp_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@
if !COMBINED_THREADS
libfftw3@PREC_SUFFIX@_omp_la_LIBADD = ../libfftw3@PREC_SUFFIX@.la
endif
endif

if HPX
libfftw3@PREC_SUFFIX@_hpx_la_SOURCES = api.c conf.c hpx.cpp \
threads.h dft-vrank-geq1.c ct.c rdft-vrank-geq1.c hc2hc.c  \
vrank-geq1-rdft2.c f77api.c f77funcs.h
libfftw3@PREC_SUFFIX@_hpx_la_CXXFLAGS = $(HPX_CXXFLAGS)
libfftw3@PREC_SUFFIX@_hpx_la_CFLAGS = $(AM_CFLAGS)
libfftw3@PREC_SUFFIX@_hpx_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@
if !COMBINED_THREADS
libfftw3@PREC_SUFFIX@_hpx_la_LIBADD = ../libfftw3@PREC_SUFFIX@.la 
endif
endif
