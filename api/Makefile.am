AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft		\
-I$(top_srcdir)/rdft -I$(top_srcdir)/reodft -I$(top_srcdir)/threads

include_HEADERS = fftw3.h
noinst_LTLIBRARIES = libapi.la

libapi_la_SOURCES = apiplan.c configure.c dfthelp.c execute.c		\
execute-dft.c execute-dft-c2r.c execute-dft-r2c.c			\
export-wisdom-to-file.c export-wisdom-to-string.c export-wisdom.c	\
forget-wisdom.c import-system-wisdom.c import-wisdom-from-file.c	\
import-wisdom-from-string.c import-wisdom.c mapflags.c			\
mkprinter-file.c mktensor-iodims.c plan-dft-1d.c plan-dft-2d.c		\
plan-dft-3d.c plan-dft.c plan-guru-dft.c plan-many-dft.c		\
plan-many-dft-c2r.c plan-dft-c2r.c plan-dft-c2r-1d.c plan-dft-c2r-2d.c	\
plan-dft-c2r-3d.c plan-many-dft-r2c.c plan-dft-r2c.c plan-dft-r2c-1d.c	\
plan-dft-r2c-2d.c plan-dft-r2c-3d.c plan-guru-dft-c2r.c			\
plan-guru-dft-r2c.c plan-with-nthreads.c print-plan.c the-planner.c	\
rdft2-pad.c api.h fftw3.h
