AUTOMAKE_OPTIONS=gnu
lib_LTLIBRARIES = lib@PRECISION@fftw3.la

# recompile genfft if maintainer mode is true
if MAINTAINER_MODE
GENFFT = genfft genfft-k7
else
GENFFT =
endif

SUBDIRS=$(GENFFT) kernel dft libbench . tests support
EXTRA_DIST=COPYRIGHT bootstrap.sh CONVENTIONS

lib@PRECISION@fftw3_la_SOURCES = 

lib@PRECISION@fftw3_la_LIBADD =				\
	kernel/libkernel.la				\
	dft/libdft.la					\
	dft/codelets/standard/libdft_standard.la	\
	dft/codelets/inplace/libdft_inplace.la		\
	dft/codelets/k7/libdft_k7.la

