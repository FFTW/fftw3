AM_CPPFLAGS = -I$(top_srcdir)/simd
noinst_LTLIBRARIES = libkernel.la
include_HEADERS = fftw3.h sfftw3.h dfftw3.h lfftw3.h

libkernel_la_SOURCES = align.c alloc.c assert.c awake.c debug.c		\
dotens.c dotens2.c minmax.c ops.c plan.c planner-naive.c		\
planner-score.c planner.c primes.c print.c printers.c problem.c scan.c	\
scanners.c solver.c solvtab.c square.c stride.c tensor.c timer.c	\
traverse.c twiddle.c version.c cycle.h dfftw3.h fftw3.h ifftw.h		\
lfftw3.h sfftw3.h
