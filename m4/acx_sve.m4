dnl @synopsis ACX_SVE([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl @summary figure out whether a simple SVE program can be compiled
dnl @category InstalledPackages
dnl
dnl This macro tries to compile a simple SVE program that uses
dnl the ACLE SVE extensions.
dnl
dnl ACTION-IF-FOUND is a list of shell commands to run if a SVE
dnl program can be compiled, and ACTION-IF-NOT-FOUND is a list of commands
dnl to run it cannot.
dnl
dnl @version 2024-04-15
dnl @license GPLWithACException
dnl @author Gilles Gouaillardet <gilles@rist.or.jp>

AC_DEFUN([ACX_SVE], [

   AC_MSG_CHECKING([whether a SVE program can be compiled])
   AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <arm_sve.h>]],
      [[#if defined(__GNUC__) && !defined(__ARM_FEATURE_SVE)
#error compiling without SVE support
#endif]])],[AC_MSG_RESULT([yes])
            $1],
       [AC_MSG_RESULT([no])
        $2])
])dnl ACX_SVE
