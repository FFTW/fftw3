dnl Process this file with autoconf to produce a configure script.
AC_INIT(kernel/fftw3.h)
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE(fftw, 3.0)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

dnl configure options
AC_ARG_ENABLE(debug, [  --enable-debug          compile fftw with extra runtime checks for debugging], ok=$enableval, ok=no)
if test "$ok" = "yes"; then
	AC_DEFINE(FFTW_DEBUG,1,[Define to enable extra FFTW debugging code.])
fi

AC_ARG_ENABLE(single, [  --enable-single        compile fftw in single precision], ok=$enableval, ok=no)
if test "$ok" = "yes"; then
	AC_DEFINE(FFTW_SINGLE,1,[Define to compile in single precision.])
	AC_DEFINE(BENCHFFT_SINGLE,1,[Define to compile in single precision.])
	PRECISION=s
else
	PRECISION=d
fi
AM_CONDITIONAL(SINGLE, test "$ok" = "yes")
AC_SUBST(PRECISION)

AC_ARG_ENABLE(k7, [  --enable-k7         enable AMD K7 optimizations], k7_mode=$enableval, k7_mode=no)
if test "$k7_mode" = "yes"; then
	AC_DEFINE(K7_MODE,1,[Define to enable AMD K7 optimizations.])
fi
AM_CONDITIONAL(K7_MODE, test "$k7_mode" = "yes")

OCAML_INIT_PATHS

dnl Checks for programs.
AC_PROG_CC
AMX_PROG_AS     dnl AM_PROG_AS breaks libtool
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

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([libintl.h malloc.h stddef.h stdlib.h string.h strings.h sys/time.h unistd.h])
dnl Header file for enabling asm() on Digital Unix  
AC_CHECK_HEADERS(c_asm.h) 

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_CHECK_TYPES([uint])
AC_HEADER_TIME

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_LIB(m, sin)
AC_CHECK_FUNCS(BSDgettimeofday)
AC_CHECK_FUNCS(gettimeofday)
AC_CHECK_FUNCS(gethrtime)
AC_CHECK_FUNCS(drand48)
AC_CHECK_FUNCS(hypot)
AC_CHECK_FUNCS(sqrt)
AC_CHECK_FUNCS(memset)
dnl high-resolution clock on SGI machines
AC_CHECK_FUNCS(clock_gettime)  

AC_MSG_CHECKING([for isnan])
AC_TRY_LINK([#include <math.h>
], if (!isnan(3.14159)) isnan(2.7183);, ok=yes, ok=no)
if test "$ok" = "yes"; then
	AC_DEFINE(HAVE_ISNAN,1,[Define if the isnan() function/macro is available.])
fi
AC_MSG_RESULT(${ok})

dnl TODO
ACX_GCC_ALIGNS_STACK()


dnl check for optimization options
ACX_PROG_CC_MAXOPT(fftw)

dnl override CFLAGS selection when debugging
if test "${enable_debug}" = "yes"; then
	CFLAGS="-g"
fi

AC_DEFINE_UNQUOTED(FFTW_CC, "$CC $CFLAGS", [C compiler name and flags])

AC_OUTPUT([
   Makefile
   support/Makefile
   genfft/Makefile
   genfft-k7/Makefile
   kernel/Makefile

   dft/Makefile
   dft/codelets/Makefile
   dft/codelets/standard/Makefile
   dft/codelets/inplace/Makefile
   dft/codelets/k7/Makefile

   libbench/Makefile
   tests/Makefile

])
