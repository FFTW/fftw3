AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft	\
-I$(top_srcdir)/rdft -I$(top_srcdir)/api -I$(top_srcdir)/tests	\
-I$(top_srcdir)/libbench2

if MPI
lib_LTLIBRARIES = libfftw3@PREC_SUFFIX@_mpi.la
include_HEADERS = fftw3-mpi.h
noinst_PROGRAMS = mpi-bench
endif

CC=@MPICC@

EXTRADIST = testsched.c README

TRANSPOSE_SRC = transpose-alltoall.c transpose-pairwise.c transpose-recurse.c transpose-problem.c transpose-solve.c mpi-transpose.h
DFT_SRC = dft-serial.c dft-rank-geq2.c dft-rank-geq2-transposed.c dft-rank1.c dft-rank1-bigvec.c dft-problem.c dft-solve.c mpi-dft.h
RDFT_SRC = rdft-serial.c rdft-rank-geq2.c rdft-rank-geq2-transposed.c rdft-rank1-bigvec.c rdft-problem.c rdft-solve.c mpi-rdft.h
RDFT2_SRC = rdft2-serial.c rdft2-rank-geq2.c rdft2-problem.c rdft2-solve.c mpi-rdft2.h
SRC = any-true.c api.c block.c choose-radix.c conf.c dtensor.c fftw3-mpi.h ifftw-mpi.h rearrange.c wisdom-api.c

libfftw3@PREC_SUFFIX@_mpi_la_SOURCES = $(SRC) $(TRANSPOSE_SRC) $(DFT_SRC) $(RDFT_SRC) $(RDFT2_SRC)

libfftw3@PREC_SUFFIX@_mpi_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@

if SMP
if !COMBINED_THREADS
LIBFFTWTHREADS = $(top_builddir)/threads/libfftw3@PREC_SUFFIX@_threads.la
endif
else
LIBFFTWTHREADS =
endif

mpi_bench_SOURCES = mpi-bench.c $(top_srcdir)/tests/fftw-bench.c $(top_srcdir)/tests/hook.c
mpi_bench_LDADD = libfftw3@PREC_SUFFIX@_mpi.la $(LIBFFTWTHREADS) $(top_builddir)/libfftw3@PREC_SUFFIX@.la $(top_builddir)/libbench2/libbench2.a $(MPILIBS) $(THREADLIBS)
