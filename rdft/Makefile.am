AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft
SUBDIRS = codelets

noinst_LTLIBRARIES = librdft.la

RDFT2 = buffered2.c direct2.c nop2.c rank0-rdft2.c rank-geq2-rdft2.c	\
plan2.c problem2.c solve2.c vrank-geq1-rdft2.c rdft2-radix2.c		\
rdft2-tensor-max-index.c rdft2-inplace-strides.c rdft2-strides.c

librdft_la_SOURCES = hc2hc.h hc2hc.c dft-r2hc.c dht-r2hc.c dht-rader.c	\
buffered.c codelet-rdft.h conf.c direct.c generic.c hc2hc-common.c	\
hc2hc-direct.c hc2hc-directbuf.c hc2hc-generic.c khc2hc.c khc2r.c	\
kr2hc.c kr2r.c indirect.c nop.c plan.c problem.c rank0.c rank-geq2.c	\
rdft.h rdft-dht.c solve.c vrank-geq1.c vrank3-transpose.c $(RDFT2)
