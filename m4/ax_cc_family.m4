dnl @synopsis AX_CC_FAMILY
dnl
dnl Determine the ``family'' of the C compiler, e.g., gnu, icc, xlc,
dnl sunpro, etc.

AC_DEFUN([AX_CC_FAMILY],
[
  AC_MSG_CHECKING([for C compiler family])
  ax_cv_cc_family=unknown
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM(, [return __GNUC__;])],
                    [ax_cv_cc_family=gnu])
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM(, [return __xlc__;])],
                    [ax_cv_cc_family=xlc])
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM(, [return __ICC;])],
                    [ax_cv_cc_family=icc])
  AC_MSG_RESULT($ax_cv_cc_family)
])

