AUTOMAKE_OPTIONS=gnu

# recompile genfft if maintainer mode is true
if MAINTAINER_MODE
GENFFT=genfft
else
GENFFT=
endif

SUBDIRS=$(GENFFT) codelets kernel dft
EXTRA_DIST=COPYRIGHT bootstrap.sh

lib_LTLIBRARIES = libfftw.la

libfftw_la_SOURCES = 

libfftw_la_LIBADD =				\
	kernel/libkernel.la			\
	dft/libdft.la				\
	codelets/libcodelets.la

