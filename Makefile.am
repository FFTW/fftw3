AUTOMAKE_OPTIONS=gnu
lib_LTLIBRARIES = lib@PRECISION@fftw3.la

# recompile genfft if maintainer mode is true
if MAINTAINER_MODE
GENFFT = genfft genfft-k7
else
GENFFT =
endif

# distribute file CLASSIC-MODE if classic mode is true
if CLASSIC_MODE
CLASSIC_DIST = CLASSIC-MODE
distdir = $(PACKAGE)-$(VERSION)-classic
else
CLASSIC_DIST = 
distdir = $(PACKAGE)-$(VERSION)-pro
endif

SUBDIRS=$(GENFFT) kernel dft libbench . tests support
EXTRA_DIST=COPYRIGHT bootstrap.sh CONVENTIONS $(CLASSIC_DIST)

lib@PRECISION@fftw3_la_SOURCES = 

lib@PRECISION@fftw3_la_LIBADD =				\
	kernel/libkernel.la				\
	dft/libdft.la					\
	dft/codelets/standard/libdft_standard.la	\
	dft/codelets/inplace/libdft_inplace.la

