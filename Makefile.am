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

# when using combined thread libraries (necessary on Windows), we want
# to build threads/ first, because libfftw3_threads is added to
# libfftw3.
#
# Otherwise, we want to build libfftw3_threads after libfftw3
# so that we can track the fact that libfftw3_threads depends upon
# libfftw3.
#
# This is the inescapable result of combining three bad ideas
# (threads, Windows, and shared libraries).
#
if COMBINED_THREADS
CHICKEN_EGG=threads .
else
CHICKEN_EGG=. threads
endif

SUBDIRS=support $(GENFFT) kernel simd-support dft rdft reodft api	\
libbench2 $(CHICKEN_EGG) tests mpi doc tools m4
EXTRA_DIST=COPYRIGHT bootstrap.sh CONVENTIONS fftw.pc.in

SIMD_LIBS =						\
	simd-support/libsimd_support.la			\
	simd-support/libsimd_sse2_nonportable.la

if HAVE_SSE2
SSE2_LIBS = dft/simd/sse2/libdft_sse2_codelets.la	\
rdft/simd/sse2/librdft_sse2_codelets.la
endif

if HAVE_AVX
AVX_LIBS = dft/simd/avx/libdft_avx_codelets.la	\
rdft/simd/avx/librdft_avx_codelets.la
endif

if HAVE_ALTIVEC
ALTIVEC_LIBS = dft/simd/altivec/libdft_altivec_codelets.la	\
rdft/simd/altivec/librdft_altivec_codelets.la
endif

if HAVE_NEON
NEON_LIBS = dft/simd/neon/libdft_neon_codelets.la	\
rdft/simd/neon/librdft_neon_codelets.la
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
	dft/scalar/libdft_scalar.la			\
	dft/scalar/codelets/libdft_scalar_codelets.la	\
	rdft/librdft.la					\
	rdft/scalar/librdft_scalar.la			\
	rdft/scalar/r2cf/librdft_scalar_r2cf.la		\
	rdft/scalar/r2cb/librdft_scalar_r2cb.la		\
	rdft/scalar/r2r/librdft_scalar_r2r.la		\
	reodft/libreodft.la				\
	api/libapi.la					\
        $(SIMD_LIBS) $(SSE2_LIBS) $(AVX_LIBS) $(ALTIVEC_LIBS) $(NEON_LIBS)    	\
	$(COMBINED_THREADLIBS)

if QUAD
# cannot use -no-undefined since dependent on libquadmath
libfftw3@PREC_SUFFIX@_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@
else
libfftw3@PREC_SUFFIX@_la_LDFLAGS = -no-undefined -version-info	\
@SHARED_VERSION_INFO@
endif

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
