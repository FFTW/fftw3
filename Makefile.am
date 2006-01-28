OPTIONS_AUTOMAKE=gnu
lib_LTLIBRARIES = libfftw3@PREC_SUFFIX@.la

# recompile genfft if maintainer mode is true
if MAINTAINER_MODE
GENFFT = genfft genfft-k7
else
GENFFT =
endif

ACLOCAL_AMFLAGS=-I ./m4

SUBDIRS=support $(GENFFT) kernel simd dft rdft reodft threads api	\
libbench2 . tests doc tools
EXTRA_DIST=COPYRIGHT bootstrap.sh CONVENTIONS fftw.pc.in m4

if HAVE_SIMD
SIMD_LIBS=						\
	simd/libsimd.la					\
	simd/nonportable/libsimd_nonportable.la		\
	dft/simd/libdft_simd.la				\
	dft/simd/codelets/libdft_codelets_simd.la	
else
SIMD_LIBS=
endif

if HAVE_K7
K7_LIBS=							\
        dft/k7/libdft_k7.la					\
	dft/k7/codelets/libdft_codelets_k7.la		
else
K7_LIBS=
endif

if THREADS
if COMBINED_THREADS
COMBINED_THREADLIBS=threads/libfftw3@PREC_SUFFIX@_threads.la
endif
endif

libfftw3@PREC_SUFFIX@_la_SOURCES = 

libfftw3@PREC_SUFFIX@_la_LIBADD =			\
	kernel/libkernel.la				\
	dft/libdft.la					\
	dft/codelets/libdft_codelets.la			\
	dft/codelets/standard/libdft_standard.la	\
	rdft/librdft.la					\
	rdft/codelets/librdft_codelets.la		\
	rdft/codelets/r2hc/librdft_codelets_r2hc.la	\
	rdft/codelets/hc2r/librdft_codelets_hc2r.la	\
	rdft/codelets/r2r/librdft_codelets_r2r.la	\
	reodft/libreodft.la				\
	api/libapi.la					\
        $(SIMD_LIBS) $(K7_LIBS) $(COMBINED_THREADLIBS)

libfftw3@PREC_SUFFIX@_la_LDFLAGS = -no-undefined -version-info @SHARED_VERSION_INFO@

fftw3@PREC_SUFFIX@.pc: fftw.pc
	cp -f fftw.pc fftw3@PREC_SUFFIX@.pc
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = fftw3@PREC_SUFFIX@.pc

WISDOM_DIR = /etc/fftw
WISDOM = wisdom@PREC_SUFFIX@

WISDOM_TIME=12 # default to 12-hour limit, i.e. overnight
WISDOM_FLAGS=--verbose --canonical --time-limit=$(WISDOM_TIME)

wisdom:
	tools/fftw@PREC_SUFFIX@-wisdom -o $@ $(WISDOM_FLAGS)

install-wisdom: wisdom
	$(mkinstalldirs) $(WISDOM_DIR)
	$(INSTALL_DATA) wisdom $(WISDOM_DIR)/$(WISDOM)
