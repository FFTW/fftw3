AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft -I$(top_srcdir)/rdft

if THREADS
SOLVERS = dft-vrank-geq1.c ct-dit.c rdft-vrank-geq1.c hc2hc-dit.c	\
hc2hc-dif.c vrank-geq1-rdft2.c
else
SOLVERS = 
endif

EXTRA_DIST = dft-vrank-geq1.c ct-dit.c rdft-vrank-geq1.c hc2hc-dit.c	\
hc2hc-dif.c vrank-geq1-rdft2.c

noinst_LTLIBRARIES = libthreads.la

libthreads_la_SOURCES = conf.c threads.c threads.h $(SOLVERS)
