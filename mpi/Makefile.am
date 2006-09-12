AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft	\
-I$(top_srcdir)/rdft -I$(top_srcdir)/api

if MPI
lib_LTLIBRARIES = libfftw3@PREC_SUFFIX@_mpi.la
include_HEADERS = fftw3_mpi.h
endif

CC=@MPICC@

EXTRADIST = testsched.c

libfftw3@PREC_SUFFIX@_mpi_la_SOURCES = api.c block.c conf.c mpi_transpose_alltoall.c mpi_transpose_inplace.c problem.c tproblem.c tsolve.c fftw3_mpi.h ifftw_mpi.h mpi_dft.h mpi_transpose.h

libfftw3@PREC_SUFFIX@_mpi_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@
