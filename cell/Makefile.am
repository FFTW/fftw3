AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft	\
-I$(top_srcdir)/simd
noinst_LTLIBRARIES = libcell.la

if HAVE_CELL
SUBDIRS=spu
SPU_STUB=spufftw-embed.lo
endif

if SINGLE
PLANS=plans-single.c
else
PLANS=plans-double.c
endif

libcell_la_SOURCES = cell.c conf.c copy.c dft-direct-cell.c	\
transpose.c $(PLANS) fftw-cell.h
libcell_la_LIBADD = $(SPU_STUB)

# HACK ALERT:
spufftw-embed.lo: conf.lo spu/spufftw
	embedspu -m32 spu_fftw spu/spufftw spufftw-embed.o
	sed -e s+conf+spufftw-embed+g <$< >$@
