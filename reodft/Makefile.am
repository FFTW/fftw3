AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/rdft
SUBDIRS = 

noinst_LTLIBRARIES = libreodft.la

EXTRA_DIST = reodft11e-r2hc.c # no longer used due to numerical problems

libreodft_la_SOURCES = conf.c reodft.h redft00e-r2hc.c	\
reodft010e-r2hc.c rodft00e-r2hc.c reodft11e-radix2.c	\
reodft11e-r2hc-odd.c
