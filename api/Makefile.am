AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft		\
-I$(top_srcdir)/rdft -I$(top_srcdir)/reodft

include_HEADERS = fftw3.h fftw3.f
noinst_LTLIBRARIES = libapi.la

libapi_la_SOURCES = apiplan.c configure.c execute.c execute-dft.c	\
execute-dft-c2r.c execute-dft-r2c.c execute-r2r.c			\
export-wisdom-to-file.c export-wisdom-to-string.c export-wisdom.c	\
extract-reim.c flops.c forget-wisdom.c import-system-wisdom.c		\
import-wisdom-from-file.c import-wisdom-from-string.c import-wisdom.c	\
mapflags.c mkprinter-file.c mktensor-iodims.c mktensor-rowmajor.c	\
plan-dft-1d.c plan-dft-2d.c plan-dft-3d.c plan-dft.c plan-guru-dft.c	\
plan-many-dft.c plan-many-dft-c2r.c plan-dft-c2r.c plan-dft-c2r-1d.c	\
plan-dft-c2r-2d.c plan-dft-c2r-3d.c plan-many-dft-r2c.c plan-dft-r2c.c	\
plan-dft-r2c-1d.c plan-dft-r2c-2d.c plan-dft-r2c-3d.c			\
plan-guru-dft-c2r.c plan-guru-dft-r2c.c map-r2r-kind.c plan-many-r2r.c	\
plan-r2r-1d.c plan-r2r-2d.c plan-r2r-3d.c plan-r2r.c plan-guru-r2r.c	\
print-plan.c the-planner.c rdft2-pad.c version.c api.h fftw3.h		\
execute-split-dft-c2r.c execute-split-dft-r2c.c execute-split-dft.c	\
plan-guru-split-dft-c2r.c plan-guru-split-dft-r2c.c			\
plan-guru-split-dft.c f77api.c f77funcs.h x77.h

BUILT_SOURCES = fftw3.f

# convert constants to F77 PARAMETER statements
fftw3.f: fftw3.h
	rm -f $@
	perl -pe 's/([A-Z0-9_]+)=([0-9]+)/\n      INTEGER \1\n      PARAMETER (\1=\2)\n/g' $< |egrep 'PARAMETER|INTEGER' > $@
	perl -pe 's/#define +([A-Z0-9_]+) +\(([+-]?[0-9]+)U?\)/\n      INTEGER \1\n      PARAMETER (\1=\2)\n/g' $< |egrep 'PARAMETER|INTEGER' >> $@
	perl -pe 'if (/#define +([A-Z0-9_]+) +\(([0-9]+)U? *<< *([0-9]+)\)/) { print "\n      INTEGER $$1\n      PARAMETER ($$1=",$$2 << $$3,")\n"; }' $< |egrep 'PARAMETER|INTEGER' >> $@
