AUTOMAKE_OPTIONS=gnu
lib_LTLIBRARIES = lib@PRECISION@fftw3.la

# recompile genfft if maintainer mode is true
if MAINTAINER_MODE
GENFFT = genfft
else
GENFFT =
endif

# distribute file RESEARCH-MODE if research mode is true
if RESEARCH_MODE
RESEARCH_DIST = RESEARCH-MODE
distdir = $(PACKAGE)-research-$(VERSION)
else
RESEARCH_DIST = 
distdir = $(PACKAGE)-$(VERSION)
endif

SUBDIRS=$(GENFFT) kernel dft libbench . tests
EXTRA_DIST=COPYRIGHT bootstrap.sh CONVENTIONS $(RESEARCH_DIST)

lib@PRECISION@fftw3_la_SOURCES = 

lib@PRECISION@fftw3_la_LIBADD =				\
	kernel/libkernel.la				\
	dft/libdft.la					\
	dft/codelets/standard/libdft_standard.la	\
	dft/codelets/inplace/libdft_inplace.la

