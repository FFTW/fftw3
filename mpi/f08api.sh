#! /bin/sh

# Script to generate the fftw3_mpi Fortran 2008 module from the
# fftw3-mpi.h header file, using mpi_f08 type-safe MPI bindings.

# This is designed so that the Fortran caller can do:
#   use fftw3_mpi
# and then call the C FFTW MPI functions directly, with TYPE(MPI_Comm)
# instead of an integer communicator handle.

echo "! Generated automatically.  DO NOT EDIT!"
echo

echo "module fftw3_mpi"
echo "  use, intrinsic :: iso_c_binding"
echo "  use mpi_f08, only: MPI_Comm"
echo "  implicit none"
echo

echo "  include 'fftw3.f03'"
echo

# Extract constants
perl -pe 's/#define +([A-Z0-9_]+) +\(([+-]?[0-9]+)U?\)/\n  integer\(C_INTPTR_T\), parameter :: \1 = \2\n/g' < fftw3-mpi.h | grep 'integer(C_INTPTR_T)'
perl -pe 'if (/#define +([A-Z0-9_]+) +\(([0-9]+)U? *<< *([0-9]+)\)/) { print "\n  integer\(C_INT\), parameter :: $1 = ",$2 << $3,"\n"; }' < fftw3-mpi.h | grep 'integer(C_INT)'

# Extract function declarations
for p in $*; do
    if test "$p" = "d"; then p=""; fi

    echo
    cat <<EOF
  type, bind(C) :: fftw${p}_mpi_ddim
     integer(C_INTPTR_T) n, ib, ob
  end type fftw${p}_mpi_ddim
EOF

    echo
    echo "  interface"
    grep -v 'mpi.h' fftw3-mpi.h | gcc -I../api -D__GNUC__=5 -D__i386__ -E - |grep "fftw${p}_mpi_init" |tr ';' '\n' | perl ../api/genf08.pl
    echo "  end interface"

done

echo "end module"