AC_DEFUN([AMX_PROG_AS],
[# By default we simply use the C compiler to build assembly code.
AC_REQUIRE([AC_PROG_CC])
AS="$CC"
ASFLAGS=""
AC_SUBST(AS)
AC_SUBST(ASFLAGS)
CCAS="$CC"
CCASFLAGS=""
AC_SUBST(CCAS)
AC_SUBST(CCASFLAGS)])
