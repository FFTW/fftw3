AUTOMAKE_OPTIONS=gnu
lib_LTLIBRARIES = lib@PRECISION@fftw3.la

# recompile genfft if maintainer mode is true
if MAINTAINER_MODE
GENFFT = genfft genfft-k7
else
GENFFT =
endif

SUBDIRS=support $(GENFFT) kernel simd dft rdft libbench . tests
EXTRA_DIST=COPYRIGHT bootstrap.sh CONVENTIONS

lib@PRECISION@fftw3_la_SOURCES = 

lib@PRECISION@fftw3_la_LIBADD =				\
	kernel/libkernel.la				\
	simd/libsimd.la					\
	dft/libdft.la					\
	dft/codelets/libdft_codelets.la			\
	dft/codelets/standard/libdft_standard.la	\
	dft/codelets/inplace/libdft_inplace.la		\
	dft/k7/libdft_k7.la				\
	dft/k7/codelets/libdft_codelets_k7.la		\
	dft/simd/libdft_simd.la				\
	dft/simd/codelets/libdft_codelets_simd.la

