AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft		\
-I$(top_srcdir)/rdft -I$(top_srcdir)/reodft -I$(top_srcdir)/threads

include_HEADERS = fftw3.h
noinst_LTLIBRARIES = libapi.la

libapi_la_SOURCES = apiplan.c configure.c dfthelp.c execute-dft.c	\
execute.c export-wisdom-to-file.c export-wisdom-to-string.c		\
export-wisdom.c forget-wisdom.c import-system-wisdom.c			\
import-wisdom-from-file.c import-wisdom-from-string.c import-wisdom.c	\
mapflags.c mkprinter-file.c plan-dft-1d.c plan-dft-2d.c plan-dft-3d.c	\
plan-dft.c print-plan.c the-planner.c api.h fftw3.h
