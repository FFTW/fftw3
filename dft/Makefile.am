AM_CPPFLAGS = -I$(top_srcdir)/kernel
SUBDIRS = codelets

noinst_LTLIBRARIES = libdft.la

libdft_la_SOURCES = ct-dit.c ct.c direct.c kdft-dit.c kdft.c plan.c	\
problem.c solve.c ct.h dft.h vecloop.c rank0.c conf.c codelet.h		\
rank_geq2.c

libdft_la_LIBADD = codelets/libdft_codelets.la
