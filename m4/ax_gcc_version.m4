dnl @synopsis AX_GCC_VERSION(MAJOR, MINOR, [ACTION-SUCCESS], [ACTION-FAILURE])
dnl
dnl Check whether we are using gcc and, if so, whether its version
dnl is at least MAJOR.MINOR.
dnl
dnl ACTION-SUCCESS/ACTION-FAILURE are shell commands to execute on
dnl success/failure.
dnl
dnl @version $Id: ax_gcc_version.m4,v 1.1 2004-10-27 17:14:22 stevenj Exp $
dnl @author Steven G. Johnson <stevenj@alum.mit.edu> and Matteo Frigo.
AC_DEFUN([AX_GCC_VERSION],
[
AC_REQUIRE([AC_PROG_CC])
AC_CACHE_CHECK(whether we are using gcc $1.$2 or later, ax_cv_gcc_$1_$2,
[
ax_cv_gcc_$1_$2=no
if test "$GCC" = "yes"; then
dnl The semicolon after "yes" below is to pacify NeXT's syntax-checking cpp.
AC_EGREP_CPP(yes, [
#ifdef __GNUC__
#  if (__GNUC__ > $1) || (__GNUC__ == $1 && __GNUC_MINOR__ >= $2)
     yes;
#  endif
#endif
], [ax_cv_gcc_$1_$2=yes])
fi
])
if test "$ax_cv_gcc_$1_$2" = yes; then
	m4_default([$3], :)
else
	m4_default([$4], :)
fi
])

