AM_CPPFLAGS = -I$(top_srcdir)/kernel
SUBDIRS = codelets simd k7

noinst_LTLIBRARIES = libdft.la

libdft_la_SOURCES = buffered.c conf.c ct-dif.c ct-dit.c ct-ditbuf.c	\
ct-ditf.c ct.c direct.c generic.c indirect.c kdft-dif.c kdft-difsq.c	\
kdft-dit.c kdft.c nop.c plan.c problem.c rader.c rader-omega.c		\
rank-geq2.c rank0.c solve.c vrank-geq1.c vrank2-transpose.c		\
vrank3-transpose.c codelet-dft.h ct.h dft.h zero.c
