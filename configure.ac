dnl Process this file with autoconf to produce a configure script.
AC_INIT(fftw, 3.0, fftw@fftw.org)
AC_CONFIG_SRCDIR(kernel/fftw3.h)
SHARED_VERSION_INFO="3:0:0"

AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE
AC_SUBST(SHARED_VERSION_INFO)
AM_ENABLE_SHARED(no) dnl to hell with shared libraries

dnl configure options
AC_ARG_ENABLE(debug, [AC_HELP_STRING([--enable-debug],[compile fftw with extra runtime checks for debugging])], ok=$enableval, ok=no)
if test "$ok" = "yes"; then
	AC_DEFINE(FFTW_DEBUG,1,[Define to enable extra FFTW debugging code.])
fi

AC_ARG_ENABLE(single, [AC_HELP_STRING([--enable-single],[compile fftw in single precision])], ok=$enableval, ok=no)
if test "$ok" = "yes"; then
	AC_DEFINE(FFTW_SINGLE,1,[Define to compile in single precision.])
	AC_DEFINE(BENCHFFT_SINGLE,1,[Define to compile in single precision.])
	PRECISION=s
else
	PRECISION=d
fi
AM_CONDITIONAL(SINGLE, test "$ok" = "yes")
AC_SUBST(PRECISION)

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

AC_ARG_ENABLE(k7, [AC_HELP_STRING([--enable-k7],[enable AMD K7 optimizations])], have_k7=$enableval, have_k7=no)
if test "$PRECISION" != "s"; then have_k7=no; fi
if test "$have_k7" = "yes"; then
	AC_DEFINE(HAVE_K7,1,[Define to enable AMD K7 optimizations.])
fi
AM_CONDITIONAL(HAVE_K7, test "$have_k7" = "yes")

have_simd="no"

AC_ARG_ENABLE(sse, [AC_HELP_STRING([--enable-sse],[enable SSE optimizations])], have_sse=$enableval, have_sse=no)
if test "$have_sse" = "yes"; then
	AC_DEFINE(HAVE_SSE,1,[Define to enable SSE optimizations.])
	have_simd="yes"
fi

AC_ARG_ENABLE(sse2, [AC_HELP_STRING([--enable-sse2],[enable SSE2 optimizations])], have_sse2=$enableval, have_sse2=no)
if test "$have_sse2" = "yes"; then
	AC_DEFINE(HAVE_SSE2,1,[Define to enable SSE2 optimizations.])
	have_simd="yes"
fi

AC_ARG_ENABLE(altivec, [AC_HELP_STRING([--enable-altivec],[enable Altivec optimizations])], have_altivec=$enableval, have_altivec=no)
if test "$have_altivec" = "yes"; then
	AC_DEFINE(HAVE_ALTIVEC,1,[Define to enable Altivec optimizations.])
	have_simd="yes"
fi

AC_ARG_ENABLE(3dnow, [AC_HELP_STRING([--enable-3dnow],[enable 3DNow! optimizations])], have_3dnow=$enableval, have_3dnow=no)
if test "$have_3dnow" = "yes"; then
	AC_DEFINE(HAVE_3DNOW,1,[Define to enable 3DNow! optimizations.])
	have_simd="yes"
fi

if test "$PRECISION" = "l" && test "$have_simd" = "yes"; then
	AC_MSG_ERROR([cannot use SIMD optimizations with long double])
fi

AM_CONDITIONAL(HAVE_SIMD, test "$have_simd" = "yes")

AC_ARG_ENABLE(unsafe-mulmod, [AC_HELP_STRING([--enable-unsafe-mulmod],[risk overflow for large prime sizes])], enable_unsafe_mulmod=$enableval, enable_unsafe_mulmod=no)
if test "$enable_unsafe_mulmod" = "yes"; then
        AC_DEFINE([FFTW_ENABLE_UNSAFE_MULMOD],1,[enable fast, unsafe modular multiplications, risking overflow for large prime sizes])
fi
OCAML_INIT_PATHS

dnl Checks for programs.
AC_PROG_CC
AMX_PROG_AS     dnl AM_PROG_AS breaks at least one version of libtool
AC_PROG_CC_STDC
ACX_PROG_CC_EGCS
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AM_PROG_LIBTOOL

AC_CHECK_PROG(OCAMLC, ocamlc, ocamlc)
AC_CHECK_PROG(OCAMLOPT, ocamlopt, ocamlopt)
AC_CHECK_PROG(OCAMLDEP, ocamldep, ocamldep)

AC_SUBST(OCAMLCFLAGS)
AC_SUBST(OCAMLOPTCFLAGS)
AC_SUBST(OCAMLDEFS)
AC_SUBST(OCAMLLDFLAGS)

dnl determine CFLAGS first
ACX_PROG_CC_MAXOPT(fftw)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([libintl.h malloc.h stddef.h stdlib.h string.h strings.h sys/time.h unistd.h limits.h c_asm.h intrinsics.h])
dnl c_asm.h: Header file for enabling asm() on Digital Unix  
dnl intrinsics.h: cray unicos

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_CHECK_TYPE([uint],, 
   [AC_DEFINE([uint], [unsigned int],
          [Define to `unsigned int' if <sys/types.h> does not define.])])
AC_HEADER_TIME
AC_CHECK_TYPE([hrtime_t],[AC_DEFINE(HAVE_HRTIME_T, 1, [Define to 1 if hrtime_t is defined in <sys/time.h>])],,
[
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
])

AC_CHECK_SIZEOF(unsigned int, 0)
AC_CHECK_SIZEOF(unsigned long long, 0)

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_LIB(m, sin)
AC_CHECK_FUNCS([BSDgettimeofday gettimeofday gethrtime read_real_time time_base_to_time drand48 hypot hypotl sqrt memset memalign _mm_malloc _mm_free clock_gettime])

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
ACX_GCC_ALIGNS_STACK()

dnl override CFLAGS selection when debugging
if test "${enable_debug}" = "yes"; then
	CFLAGS="-g"
	LDFLAGS="-static"
fi

AC_DEFINE_UNQUOTED(FFTW_CC, "$CC $CFLAGS", [C compiler name and flags])
AC_DEFINE_UNQUOTED(CODELET_OPTIM, "$CODELET_OPTIM", [extra CFLAGS for codelets])

AC_OUTPUT([
   Makefile
   support/Makefile
   genfft/Makefile
   genfft-k7/Makefile
   kernel/Makefile
   simd/Makefile

   dft/Makefile
   dft/codelets/Makefile
   dft/codelets/standard/Makefile
   dft/codelets/inplace/Makefile
   dft/simd/Makefile
   dft/simd/codelets/Makefile
   dft/k7/Makefile
   dft/k7/codelets/Makefile

   rdft/Makefile
   rdft/codelets/Makefile
   rdft/codelets/r2hc/Makefile
   rdft/codelets/hc2r/Makefile

   reodft/Makefile

   libbench/Makefile
   tests/Makefile

])
