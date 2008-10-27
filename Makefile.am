OPTIONS_AUTOMAKE=gnu
lib_LTLIBRARIES = libfftw3@PREC_SUFFIX@.la

# pkgincludedir = $(includedir)/fftw3@PREC_SUFFIX@
# nodist_pkginclude_HEADERS = config.h

# recompile genfft if maintainer mode is true
if MAINTAINER_MODE
GENFFT = genfft
else
GENFFT =
endif

ACLOCAL_AMFLAGS=-I m4

SUBDIRS=support $(GENFFT) kernel simd dft rdft reodft api threads \
cell libbench2 . tests doc tools
EXTRA_DIST=COPYRIGHT bootstrap.sh CONVENTIONS fftw.pc.in m4	\
README.Cell

if HAVE_SIMD
SIMD_LIBS=							\
	simd/nonportable/libsimd_nonportable.la			\
	dft/simd/libdft_simd.la					\
	dft/simd/codelets/libdft_simd_codelets.la		\
	rdft/simd/librdft_simd.la				\
	rdft/simd/codelets/librdft_simd_codelets.la	
SIMD_OR_CELL_LIBS=simd/libsimd.la
endif

if HAVE_CELL
CELL_LIBS=cell/libcell.la
SIMD_OR_CELL_LIBS=simd/libsimd.la
endif


if SMP
if COMBINED_THREADS
COMBINED_THREADLIBS=threads/libfftw3@PREC_SUFFIX@_threads.la
endif
endif

libfftw3@PREC_SUFFIX@_la_SOURCES = 

libfftw3@PREC_SUFFIX@_la_LIBADD =			\
	kernel/libkernel.la				\
	dft/libdft.la					\
	dft/scalar/libdft_scalar.la			\
	dft/scalar/codelets/libdft_scalar_codelets.la	\
	rdft/librdft.la					\
	rdft/scalar/librdft_scalar.la			\
	rdft/scalar/r2cf/librdft_scalar_r2cf.la		\
	rdft/scalar/r2cb/librdft_scalar_r2cb.la		\
	rdft/scalar/r2r/librdft_scalar_r2r.la		\
	reodft/libreodft.la				\
	api/libapi.la					\
        $(SIMD_LIBS) $(CELL_LIBS) $(SIMD_OR_CELL_LIBS)	\
	$(COMBINED_THREADLIBS)

libfftw3@PREC_SUFFIX@_la_LDFLAGS = -no-undefined -version-info	\
@SHARED_VERSION_INFO@

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
