OPTIONS_AUTOMAKE=gnu
lib_LTLIBRARIES = libfftw3@PREC_SUFFIX@.la

# recompile genfft if maintainer mode is true
if MAINTAINER_MODE
GENFFT = genfft genfft-k7
else
GENFFT =
endif

SUBDIRS=support $(GENFFT) kernel simd dft rdft reodft libbench . tests
EXTRA_DIST=COPYRIGHT bootstrap.sh CONVENTIONS

libfftw3@PREC_SUFFIX@_la_SOURCES = 

libfftw3@PREC_SUFFIX@_la_LIBADD =				\
	kernel/libkernel.la				\
	simd/libsimd.la					\
	dft/libdft.la					\
	dft/codelets/libdft_codelets.la			\
	dft/codelets/standard/libdft_standard.la	\
	dft/codelets/inplace/libdft_inplace.la		\
	dft/k7/libdft_k7.la				\
	dft/k7/codelets/libdft_codelets_k7.la		\
	dft/simd/libdft_simd.la				\
	dft/simd/codelets/libdft_codelets_simd.la	\
	rdft/librdft.la					\
	rdft/codelets/librdft_codelets.la		\
	rdft/codelets/r2hc/librdft_codelets_r2hc.la	\
	rdft/codelets/hc2r/librdft_codelets_hc2r.la     \
	reodft/libreodft.la

libfftw3@PREC_SUFFIX@_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@