AM_CPPFLAGS = -I$(top_srcdir)/kernel
SUBDIRS = codelets

noinst_LTLIBRARIES = libdft.la

libdft_la_SOURCES = buffered.c conf.c ct-dif.c ct-dit.c ct-ditbuf.c	\
ct-ditf.c ct.c direct-k7.c direct.c indirect.c kdft-dif.c kdft-difsq.c	\
kdft-dit.c kdft-k7.c kdft.c nop.c plan.c problem.c rank-geq2.c rank0.c	\
solve.c vrank-geq1.c vrank2-transpose.c vrank3-transpose.c		\
codelet-k7.h codelet.h ct.h dft.h
