AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft	\
-I$(top_srcdir)/rdft -I$(top_srcdir)/api

if SMP
if COMBINED_THREADS
noinst_LTLIBRARIES = libfftw3@PREC_SUFFIX@_threads.la
else
lib_LTLIBRARIES = libfftw3@PREC_SUFFIX@_threads.la
endif
endif

# pkgincludedir = $(includedir)/fftw3@PREC_SUFFIX@
# pkginclude_HEADERS = threads.h

libfftw3@PREC_SUFFIX@_threads_la_SOURCES = api.c conf.c threads.c	\
openmp.c threads.h dft-vrank-geq1.c ct.c rdft-vrank-geq1.c hc2hc.c	\
vrank-geq1-rdft2.c f77api.c f77funcs.h

libfftw3@PREC_SUFFIX@_threads_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@
