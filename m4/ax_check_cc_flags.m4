dnl @synopsis AX_CHECK_CC_FLAGS(FLAGS, [CACHEVAR], [ACTION-SUCCESS], [ACTION-FAILURE])
dnl
dnl Check whether the given C compiler FLAGS work with the current $CC
dnl compiler, or whether they give an error.  (Warnings, however, are
dnl ignored.)
dnl
dnl ACTION-SUCCESS/ACTION-FAILURE are shell commands to execute on
dnl success/failure.
dnl
dnl If CACHEVAR is supplied, the result of this check is cached in the
dnl variable $ax_cv_CACHEVAR.  Otherwise (if CACHEVAR is []) the result
dnl is not cached.  (CACHEVAR must therefore be a constant string, not
dnl a shell variable, and should consist only of valid shell-identifier
dnl characters.)
dnl
dnl @version $Id: ax_check_cc_flags.m4,v 1.1 2004-10-27 17:14:22 stevenj Exp $
dnl @author Steven G. Johnson <stevenj@alum.mit.edu> and Matteo Frigo.
AC_DEFUN([AX_CHECK_CC_FLAGS],
[
AC_REQUIRE([AC_PROG_CC])
m4_if($2, [],
   [AC_MSG_CHECKING(whether $CC accepts $1)
    ax_save_CFLAGS=$CFLAGS; CFLAGS="$1"
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],
	[ax_check_cc_flags=yes], [ax_check_cc_flags=no])
    CFLAGS=$ax_save_CFLAGS
    AC_MSG_RESULT($ax_check_cc_flags)],
   [AC_CACHE_CHECK(whether $CC accepts $1, ax_cv_$2, [
	 ax_save_CFLAGS=$CFLAGS; CFLAGS="$1"
	 AC_COMPILE_IFELSE([AC_LANG_PROGRAM()], [ax_cv_$2=yes], [ax_cv_$2=no])
	 CFLAGS=$ax_save_CFLAGS])])
if test m4_if($2, [], $ax_check_cc_flags, "$ax_cv_$2") = yes; then
	m4_default([$3], :)
else
	m4_default([$4], :)
fi
])
