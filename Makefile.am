AUTOMAKE_OPTIONS=gnu

# recompile genfft if maintainer mode is true
if MAINTAINER_MODE
GENFFT=genfft
else
GENFFT=
endif

SUBDIRS=$(GENFFT) codelets kernel dft libbench
EXTRA_DIST=COPYRIGHT bootstrap.sh CONVENTIONS

lib_LTLIBRARIES = libfftw3.la

libfftw3_la_SOURCES = 

libfftw3_la_LIBADD =				\
	kernel/libkernel.la			\
	dft/libdft.la				\
	codelets/libcodelets.la

