AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft	\
-I$(top_srcdir)/rdft -I$(top_srcdir)/api -I$(top_srcdir)/tests	\
-I$(top_srcdir)/libbench2

if MPI
lib_LTLIBRARIES = libfftw3@PREC_SUFFIX@_mpi.la
include_HEADERS = fftw3_mpi.h
noinst_PROGRAMS = mpi_bench
endif

CC=@MPICC@

EXTRADIST = testsched.c

TRANSPOSE_SRC = mpi_transpose_alltoall.c mpi_transpose_inplace.c tproblem.c tsolve.c mpi_transpose.h
DFT_SRC = mpi-dft-serial.c mpi-dft-rank-geq2.c problem.c solve.c mpi_dft.h

libfftw3@PREC_SUFFIX@_mpi_la_SOURCES = api.c block.c conf.c fftw3_mpi.h ifftw_mpi.h $(TRANSPOSE_SRC) $(DFT_SRC)

libfftw3@PREC_SUFFIX@_mpi_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@

if THREADS
if !COMBINED_THREADS
LIBFFTWTHREADS = $(top_builddir)/threads/libfftw3@PREC_SUFFIX@_threads.la
endif
else
LIBFFTWTHREADS =
endif

mpi_bench_SOURCES = mpi_bench.c $(top_srcdir)/tests/fftw_bench_common.c $(top_srcdir)/tests/hook.c
mpi_bench_LDADD = libfftw3@PREC_SUFFIX@_mpi.la $(LIBFFTWTHREADS) $(top_builddir)/libfftw3@PREC_SUFFIX@.la $(top_builddir)/libbench2/libbench2.a $(MPILIBS) $(THREADLIBS)
