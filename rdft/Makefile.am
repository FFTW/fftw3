AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft
SUBDIRS = codelets

noinst_LTLIBRARIES = librdft.la

librdft_la_SOURCES = dft-r2hc.c buffered.c codelet.h conf.c		\
direct-r2hc.c generic.c khc2hc-dif.c khc2hc-dit.c khc2r.c kr2hc.c	\
hc2hc.c hc2hc.h hc2hc-dit.c hc2hc-ditbuf.c indirect.c nop.c plan.c	\
problem.c r2hc-hc2r.c rader-dht.c rader-hc2hc.c rank0.c rdft.h solve.c	\
verify.c vrank-geq1.c vrank2-transpose.c vrank3-transpose.c

