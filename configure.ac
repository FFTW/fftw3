dnl Process this file with autoconf to produce a configure script.
AC_INIT(fftw, 3.1, fftw@fftw.org)
AC_CONFIG_SRCDIR(kernel/ifftw.h)
SHARED_VERSION_INFO="4:0:1" # CURRENT:REVISION:AGE

AM_INIT_AUTOMAKE(1.6)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE
AC_SUBST(SHARED_VERSION_INFO)
AM_ENABLE_SHARED(no) dnl to hell with shared libraries
AC_CANONICAL_HOST

dnl configure options
case "${host_cpu}" in
  powerpc*) have_fma=yes;;
  ia64*) have_fma=yes;;
  *) have_fma=no;;
esac

AC_ARG_ENABLE(fma, [AC_HELP_STRING([--enable-fma],[enable optimizations for machines with fused multiply-add])], have_fma=$enableval)
if test "$have_fma"x = "yes"x; then
	AC_DEFINE(HAVE_FMA,1,[Define if you have a machine with fused multiply-add])
fi


AC_ARG_ENABLE(debug, [AC_HELP_STRING([--enable-debug],[compile fftw with extra runtime checks for debugging])], ok=$enableval, ok=no)
if test "$ok" = "yes"; then
	AC_DEFINE(FFTW_DEBUG,1,[Define to enable extra FFTW debugging code.])
	debug_malloc=yes
else
	debug_malloc=no
fi

AC_ARG_ENABLE(debug-malloc, [AC_HELP_STRING([--enable-debug-malloc],[enable malloc debugging version])], ok=$enableval, ok=$debug_malloc)
if test "$ok" = "yes"; then
	AC_DEFINE(FFTW_DEBUG_MALLOC,1,[Define to enable debugging malloc.])
fi

AC_ARG_ENABLE(debug-alignment, [AC_HELP_STRING([--enable-debug-alignment],[enable alignment debugging hacks])], ok=$enableval, ok=no)
if test "$ok" = "yes"; then
	AC_DEFINE(FFTW_DEBUG_ALIGNMENT,1,[Define to enable alignment debugging hacks.])
fi

AC_ARG_ENABLE(single, [AC_HELP_STRING([--enable-single],[compile fftw in single precision])], ok=$enableval, ok=no)
AC_ARG_ENABLE(float, [AC_HELP_STRING([--enable-float],[synonym for --enable-single])], ok=$enableval)
if test "$ok" = "yes"; then
	AC_DEFINE(FFTW_SINGLE,1,[Define to compile in single precision.])
	AC_DEFINE(BENCHFFT_SINGLE,1,[Define to compile in single precision.])
	PRECISION=s
else
	PRECISION=d
fi
AM_CONDITIONAL(SINGLE, test "$ok" = "yes")

AC_ARG_ENABLE(long-double, [AC_HELP_STRING([--enable-long-double],[compile fftw in long-double precision])], ok=$enableval, ok=no)
if test "$ok" = "yes"; then
	if test "$PRECISION" = "s"; then
		AC_MSG_ERROR([--enable-single/--enable-long-double conflict])
	fi
	AC_DEFINE(FFTW_LDOUBLE,1,[Define to compile in long-double precision.])
	AC_DEFINE(BENCHFFT_LDOUBLE,1,[Define to compile in long-double precision.])
	PRECISION=l
fi
AM_CONDITIONAL(LDOUBLE, test "$ok" = "yes")
AC_SUBST(PRECISION)

AC_ARG_ENABLE(k7, [AC_HELP_STRING([--enable-k7],[enable AMD K7 optimizations, including 3dNow!])], have_k7=$enableval, have_k7=no)
if test "$PRECISION" != "s"; then have_k7=no; fi
if test "$have_k7" = "yes"; then
	AC_DEFINE(HAVE_K7,1,[Define to enable AMD K7 optimizations.])
fi
AM_CONDITIONAL(HAVE_K7, test "$have_k7" = "yes")

have_simd="no"
simd_precision="s"

AC_ARG_ENABLE(sse, [AC_HELP_STRING([--enable-sse],[enable SSE optimizations])], have_sse=$enableval, have_sse=no)
if test "$have_sse" = "yes"; then
	AC_DEFINE(HAVE_SSE,1,[Define to enable SSE optimizations.])
	have_simd="yes"
fi

AC_ARG_ENABLE(sse2, [AC_HELP_STRING([--enable-sse2],[enable SSE2 optimizations])], have_sse2=$enableval, have_sse2=no)
if test "$have_sse2" = "yes"; then
	AC_DEFINE(HAVE_SSE2,1,[Define to enable SSE2 optimizations.])
	have_simd="yes"
	simd_precision="d"
	if test "$have_sse" = "yes"; then
		AC_MSG_ERROR([you cannot use SSE and SSE2 at the same time])
	fi
fi

AC_ARG_ENABLE(altivec, [AC_HELP_STRING([--enable-altivec],[enable Altivec optimizations])], have_altivec=$enableval, have_altivec=no)
if test "$have_altivec" = "yes"; then
	AC_DEFINE(HAVE_ALTIVEC,1,[Define to enable Altivec optimizations.])
	have_simd="yes"
fi

AC_ARG_ENABLE(3dnow, [AC_HELP_STRING([--enable-3dnow],[enable 3DNow! optimizations (fallback for non-k7-compatible)])], have_3dnow=$enableval, have_3dnow=no)
if test "$have_3dnow" = "yes"; then
	AC_DEFINE(HAVE_3DNOW,1,[Define to enable 3DNow! optimizations.])
	have_simd="yes"
fi

if test "$PRECISION" != "$simd_precision" && test "$have_simd" = "yes"; then
	AC_MSG_ERROR([requested SIMD type does not match selected precision])
fi

AM_CONDITIONAL(HAVE_SIMD, test "$have_simd" = "yes")

AC_ARG_WITH(slow-timer, [AC_HELP_STRING([--with-slow-timer],[use low-precision timers (SLOW)])], with_slow_timer=$withval, with_slow_timer=no)
if test "$with_slow_timer" = "yes"; then
	AC_DEFINE(WITH_SLOW_TIMER,1,[Use low-precision timers, making planner very slow])
fi

AC_ARG_WITH(our-malloc16, [AC_HELP_STRING([--with-our-malloc16],[use our 16-byte aligned malloc (helpful for Win32)])], with_our_malloc16=$withval, with_our_malloc16=no)
if test "$with_our_malloc16" = "yes"; then
	AC_DEFINE(WITH_OUR_MALLOC16,1,[Use our own 16-byte aligned malloc routine; mainly helpful for Windows systems lacking aligned allocation system-library routines.])
fi

AC_ARG_WITH(windows-f77-mangling, [AC_HELP_STRING([--with-windows-f77-mangling],[use common Win32 Fortran interface styles])], with_windows_f77_mangling=$withval, with_windows_f77_mangling=no)
if test "$with_windows_f77_mangling" = "yes"; then
	AC_DEFINE(WINDOWS_F77_MANGLING,1,[Use common Windows Fortran mangling styles for the Fortran interfaces.])
fi

AC_ARG_ENABLE(unsafe-mulmod, [AC_HELP_STRING([--enable-unsafe-mulmod],[risk overflow for large prime sizes])], enable_unsafe_mulmod=$enableval, enable_unsafe_mulmod=no)
if test "$enable_unsafe_mulmod" = "yes"; then
        AC_DEFINE([FFTW_ENABLE_UNSAFE_MULMOD],1,[enable fast, unsafe modular multiplications, risking overflow for large prime sizes])
fi
OCAML_INIT_PATHS

dnl compute library suffix
case "$PRECISION" in
     s) PREC_SUFFIX=f;;
     d) PREC_SUFFIX=;;
     l) PREC_SUFFIX=l;;
esac
AC_SUBST(PREC_SUFFIX)

dnl Checks for programs.
AC_PROG_CC
AX_CC_VENDOR
AMX_PROG_AS     dnl AM_PROG_AS breaks at least one version of libtool
AC_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AM_PROG_LIBTOOL

AC_CHECK_PROG(OCAMLC, ocamlc, ocamlc)
AC_CHECK_PROG(OCAMLOPT, ocamlopt, ocamlopt)
AC_CHECK_PROG(OCAMLDEP, ocamldep, ocamldep)

AC_SUBST(OCAMLCFLAGS)
AC_SUBST(OCAMLOPTCFLAGS)
AC_SUBST(OCAMLDEFS)
AC_SUBST(OCAMLLDFLAGS)

dnl determine CFLAGS first
AX_CC_MAXOPT

# some additional FFTW-specific tweaks to the flags
if test "$ac_test_CFLAGS" != "set"; then # if user didn't override CFLAGS
    if test "x$CODELET_OPTIM" = x; then

      case "${ax_cv_cc_vendor}" in
        gnu)
            case "${host_cpu}" in
	       # gcc's ``optimizations'' slow down everything on IA32.
	       # (not to mention compiler bugs)
               i?86*) CODELET_OPTIM=-O ;;
            esac

	    # disable certain other gcc ``optimizations'' that produce worse 
	    # codelets
	    AX_CHECK_COMPILER_FLAGS(-fno-schedule-insns,
		    [CODELET_OPTIM="$CODELET_OPTIM -fno-schedule-insns"])
	    AX_CHECK_COMPILER_FLAGS(-fno-web,
		    [CODELET_OPTIM="$CODELET_OPTIM -fno-web"])
	    AX_CHECK_COMPILER_FLAGS(-fno-loop-optimize,
		    [CODELET_OPTIM="$CODELET_OPTIM -fno-loop-optimize"])

	    # The gcc-3.4 inliner is totally broken; it does not inline
	    # small functions like VFMAI.
	    AX_CHECK_COMPILER_FLAGS(
		    [--param inline-unit-growth=1000],
		    [CODELET_OPTIM="$CODELET_OPTIM --param inline-unit-growth=1000"])
	    AX_CHECK_COMPILER_FLAGS(
		    [--param large-function-growth=1000],
		    [CODELET_OPTIM="$CODELET_OPTIM --param large-function-growth=1000"])
            ;;

        ibm) CODELET_OPTIM=-O;;

      esac
    fi
fi

AC_SUBST(CODELET_OPTIM)

if test "x$SIMD_CFLAGS" = x; then
if test "$GCC" = "yes"; then
    if test "$have_sse" = "yes"; then
	AX_CHECK_COMPILER_FLAGS(-msse, [SIMD_CFLAGS="-msse"],
	    [AC_MSG_ERROR([Need a version of gcc with -msse])])
    elif test "$have_sse2" = "yes"; then
	AX_CHECK_COMPILER_FLAGS(-msse2, [SIMD_CFLAGS="-msse2"],
	    [AC_MSG_ERROR([Need a version of gcc with -msse2])])
    elif test "$have_3dnow" = "yes"; then
	AX_CHECK_COMPILER_FLAGS(-m3dnow, [SIMD_CFLAGS="-m3dnow"],
	    [AC_MSG_ERROR([Need a version of gcc with -m3dnow])])
    elif test "$have_altivec" = "yes"; then
	AX_CHECK_COMPILER_FLAGS(-faltivec, [SIMD_CFLAGS="-faltivec"],
	    [AX_CHECK_COMPILER_FLAGS(-maltivec -mabi=altivec,
		[SIMD_CFLAGS="-maltivec -mabi=altivec"],
	        [AX_CHECK_COMPILER_FLAGS(-fvec, [SIMD_CFLAGS="-fvec"],
		    [AC_MSG_ERROR([Need a version of gcc with -maltivec])])])])
    fi
fi
fi
AC_SUBST(SIMD_CFLAGS)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([libintl.h malloc.h stddef.h stdlib.h string.h strings.h sys/time.h unistd.h limits.h c_asm.h intrinsics.h stdint.h mach/mach_time.h sys/sysctl.h altivec.h])
dnl c_asm.h: Header file for enabling asm() on Digital Unix  
dnl intrinsics.h: cray unicos
dnl sys/sysctl.h: MacOS X altivec detection
dnl altivec.h: required by gcc -maltivec on linux

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_CHECK_TYPE([hrtime_t],[AC_DEFINE(HAVE_HRTIME_T, 1, [Define to 1 if hrtime_t is defined in <sys/time.h>])],,
[
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
])

AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)

AC_CHECK_TYPES(uintptr_t, [], [AC_CHECK_SIZEOF(unsigned int)
AC_CHECK_SIZEOF(unsigned long)
AC_CHECK_SIZEOF(unsigned long long)
AC_CHECK_SIZEOF(void *)], [$ac_includes_default
#ifdef HAVE_STDINT_H
#  include <stdint.h>
#endif])

AC_CHECK_SIZEOF(float)
AC_CHECK_SIZEOF(double)
AC_CHECK_SIZEOF(long double)
if test $PRECISION = l; then
        if test $ac_cv_sizeof_long_double = 0; then
                AC_MSG_ERROR([long double is not a supported type with your compiler.])
	fi
fi

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_LIB(m, sin)
AC_CHECK_FUNCS([BSDgettimeofday gettimeofday gethrtime read_real_time time_base_to_time drand48 sqrt memset posix_memalign memalign _mm_malloc _mm_free clock_gettime mach_absolute_time sysctl abort])
AC_CHECK_DECLS([drand48, memalign, posix_memalign])

dnl Cray UNICOS _rtc() (real-time clock) intrinsic
AC_MSG_CHECKING([for _rtc intrinsic])
rtc_ok=yes
AC_TRY_LINK([#ifdef HAVE_INTRINSICS_H
#include <intrinsics.h>
#endif], [_rtc()], [AC_DEFINE(HAVE__RTC,1,[Define if you have the UNICOS _rtc() intrinsic.])], [rtc_ok=no])
AC_MSG_RESULT($rtc_ok)

if test "$PRECISION" = "l"; then
	AC_CHECK_FUNCS([cosl sinl tanl], [], [AC_MSG_ERROR([long-double precision requires long-double trigonometric routines])])
fi

AC_MSG_CHECKING([for isnan])
AC_TRY_LINK([#include <math.h>
], if (!isnan(3.14159)) isnan(2.7183);, ok=yes, ok=no)
if test "$ok" = "yes"; then
	AC_DEFINE(HAVE_ISNAN,1,[Define if the isnan() function/macro is available.])
fi
AC_MSG_RESULT(${ok})

dnl TODO
AX_GCC_ALIGNS_STACK()

dnl override CFLAGS selection when debugging
if test "${enable_debug}" = "yes"; then
	CFLAGS="-g"
	LDFLAGS="-static"
fi

dnl add gcc warnings, in debug/maintainer mode only
if test "$enable_debug" = yes || test "$USE_MAINTAINER_MODE" = yes; then
	if test $ac_cv_prog_gcc = yes; then
		CFLAGS="$CFLAGS -Wall -W -Wcast-qual -Wpointer-arith -Wcast-align -pedantic -Wno-long-long -Wshadow -Wbad-function-cast -Wwrite-strings -Wstrict-prototypes -Wredundant-decls -Wnested-externs" # -Wundef -Wconversion -Wmissing-prototypes -Wmissing-declarations 
	fi
fi

dnl -----------------------------------------------------------------------

AC_ARG_ENABLE(fortran, [  --disable-fortran       don't include fortran-callable wrappers], enable_fortran=$enableval, enable_fortran=yes)

if test "$enable_fortran" = "yes"; then
        AC_PROG_F77
        if test -z "$F77"; then
                enable_fortran=no
                AC_MSG_WARN([*** Couldn't find f77 compiler; using default Fortran wrappers.])
	else
		AC_F77_DUMMY_MAIN([], [enable_fortran=no
			AC_MSG_WARN([*** Couldn't figure out how to link C and Fortran; using default Fortran wrappers.])])
        fi
else
	AC_DEFINE([DISABLE_FORTRAN], 1, [Define to disable Fortran wrappers.])
fi

if test "x$enable_fortran" = xyes; then
        AC_F77_WRAPPERS
	AC_F77_FUNC(f77foo)
	AC_F77_FUNC(f77_foo)
	f77_foo2=`echo $f77foo | sed 's/77/77_/'`
	if test "$f77_foo" = "$f77_foo2"; then
		AC_DEFINE(F77_FUNC_EQUIV, 1, [Define if F77_FUNC and F77_FUNC_ are equivalent.])
	fi
fi

dnl -----------------------------------------------------------------------

AC_ARG_WITH(openmp, [AC_HELP_STRING([--with-openmp],[use OpenMP directives instead of a threads library])], with_openmp=$withval, with_openmp=no)

AC_ARG_ENABLE(threads, [AC_HELP_STRING([--enable-threads],[compile FFTW SMP threads library])], enable_threads=$enableval, enable_threads=$with_openmp)

dnl Check for threads library...
THREADLIBS=""
if test "$enable_threads" = "yes"; then
	if test "$with_openmp"x != nox; then
		AX_OPENMP([THREADLIBS=" "
		  AC_DEFINE(USING_OPENMP_THREADS, 1, [Define if we have and are using OpenMP multithreading directives])
                  CFLAGS="$CFLAGS $OPENMP_CFLAGS"],
		  [AC_MSG_ERROR([don't know how to enable OpenMP])])
	fi

	# POSIX threads, the default choice:
	if test -z "$THREADLIBS"; then
		ACX_PTHREAD([THREADLIBS="$PTHREAD_LIBS "
	                     CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
	                     CC="$PTHREAD_CC"
	                     AC_DEFINE(USING_POSIX_THREADS, 1, [Define if we have and are using POSIX threads.])])
	fi

	if test -z "$THREADLIBS"; then
		AC_MSG_CHECKING([for Win32 threads])
		AC_TRY_LINK([#include <windows.h>],
			[_beginthreadex(0,0,0,0,0,0);],
			[THREADLIBS=" "; AC_MSG_RESULT(yes)],
			[AC_MSG_RESULT(no)])
	fi

	if test -z "$THREADLIBS"; then
		AC_MSG_ERROR([couldn't find threads library for --enable-threads])
	fi
	AC_DEFINE(HAVE_THREADS, 1, [Define if we have a threads library.])
fi
AC_SUBST(THREADLIBS)
AM_CONDITIONAL(THREADS, test x"$THREADLIBS" != x)

dnl -----------------------------------------------------------------------

AC_MSG_CHECKING([whether a cycle counter is available])
save_CPPFLAGS=$CPPFLAGS
CPPFLAGS="$CPPFLAGS -I$srcdir/kernel"
AC_TRY_CPP([#include "cycle.h"
#ifndef HAVE_TICK_COUNTER
#  error No cycle counter
#endif], [ok=yes], [ok=no])
CPPFLAGS=$save_CPPFLAGS
AC_MSG_RESULT($ok)
if test $ok = no && test "x$with_slow_timer" = xno; then
	echo "***************************************************************"
	echo "WARNING: No cycle counter found.  FFTW will use ESTIMATE mode  "
	echo "         for all plans.  See the manual for more information."
	echo "***************************************************************"
fi

dnl -----------------------------------------------------------------------

AC_DEFINE_UNQUOTED(FFTW_CC, "$CC $CFLAGS", [C compiler name and flags])
AC_DEFINE_UNQUOTED(CODELET_OPTIM, "$CODELET_OPTIM", [extra CFLAGS for codelets])

AC_CONFIG_FILES([
   Makefile
   support/Makefile
   genfft/Makefile
   genfft-k7/Makefile
   kernel/Makefile
   simd/Makefile

   dft/Makefile
   dft/codelets/Makefile
   dft/codelets/standard/Makefile
   dft/simd/Makefile
   dft/simd/codelets/Makefile
   dft/k7/Makefile
   dft/k7/codelets/Makefile

   rdft/Makefile
   rdft/codelets/Makefile
   rdft/codelets/r2hc/Makefile
   rdft/codelets/hc2r/Makefile
   rdft/codelets/r2r/Makefile

   reodft/Makefile

   threads/Makefile

   api/Makefile

   libbench2/Makefile
   tests/Makefile
   doc/Makefile
   doc/FAQ/Makefile

   tools/Makefile
   tools/fftw_wisdom.1
   tools/fftw-wisdom-to-conf

   fftw.pc
])

AC_OUTPUT
