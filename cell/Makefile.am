AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft	\
-I$(top_srcdir)/simd
noinst_LTLIBRARIES = libcell.la

if HAVE_CELL
SUBDIRS=spu
SPU_STUB=spu_fftw-embed.lo
endif

if SINGLE
PLANS=plans-single.c
else
PLANS=plans-double.c
endif

libcell_la_SOURCES = cell.c conf.c dft-direct-cell.c transpose.c	\
$(PLANS) fftw-cell.h
libcell_la_LIBADD = $(SPU_STUB)

# HACK ALERT:
spu_fftw-embed.lo: conf.lo FORCE 
	(cd spu && make spu_fftw-embed.o)
	sed -e s+conf+spu/spu_fftw-embed+g <$< >$@

FORCE:



