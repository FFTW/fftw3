AM_CPPFLAGS = -I$(top_srcdir)/kernel
SUBDIRS = codelets simd k7

noinst_LTLIBRARIES = libdft.la

libdft_la_SOURCES = buffered.c conf.c ct.c ctsq.c dftw-dft.c direct.c	\
directw.c directwbuf.c directwsq.c generic.c indirect.c kdft-dif.c	\
kdft-difsq.c kdft-dit.c kdft.c nop.c plan.c problem.c problemw.c	\
rader-omega.c rader.c rank-geq2.c rank0.c solve.c vrank-geq1.c		\
vrank2-transpose.c vrank3-transpose.c zero.c codelet-dft.h dft.h
