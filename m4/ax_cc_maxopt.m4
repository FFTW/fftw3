dnl @synopsis AX_CC_MAXOPT
dnl
dnl Try to turn on "good" C optimization flags for various compilers
dnl and architectures, for some definition of "good".  (In our case,
dnl good for FFTW and hopefully for other scientific codes.)
dnl
dnl The user can override the flags by setting the CFLAGS environment
dnl variable.  The user can also specify --with-portable-binary in
dnl order to disable any optimization flags that might result in
dnl a binary that only runs on the host architecture.
dnl
dnl Note also that the flags assume that ANSI C aliasing rules are
dnl followed by the code (e.g. for gcc's -fstrict-aliasing), and that
dnl floating-point computations can be re-ordered as needed.
dnl
dnl Requires macros: AX_CHECK_COMPILER_FLAGS, AX_GCC_ARCHFLAG
dnl
dnl @version $Id: ax_cc_maxopt.m4,v 1.4 2005-01-12 03:13:24 athena Exp $
dnl @author Steven G. Johnson <stevenj@alum.mit.edu> and Matteo Frigo.
AC_DEFUN([AX_CC_MAXOPT],
[
AC_REQUIRE([AC_PROG_CC])
AC_REQUIRE([AC_CANONICAL_HOST])

AC_ARG_WITH(portable-binary, [AC_HELP_STRING([--with-portable-binary], [disable compiler optimizations that would produce unportable binaries])], 
	acx_maxopt_portable=$withval, acx_maxopt_portable=no)

# Try to determine "good" native compiler flags if none specified on command
# line
if test "$ac_test_CFLAGS" != "set"; then
  CFLAGS=""
  if test "$GCC" '!=' "yes"; then
  base_CC=`basename "$CC" | cut -d" " -f1`
  case "${host_cpu}-${host_os}" in
    alpha*linux*)  if test "$base_CC" = ccc; then
                      CFLAGS="-newc -w0 -O5 -ansi_alias -ansi_args -fp_reorder -tune host"
		      if test "x$acx_maxopt_portable" = xno; then
			 CFLAGS="$CFLAGS -arch host"
		      fi
                   fi;;

    *linux*) ;;

    *-solaris2*) if test "$base_CC" = cc; then
                    CFLAGS="-native -fast -xO5 -dalign"
		      if test "x$acx_maxopt_portable" = xyes; then
			 CFLAGS="$CFLAGS -xarch=generic"
		      fi
                 fi;;

    alpha*-osf*)  if test "$base_CC" = cc; then
                    CFLAGS="-newc -w0 -O5 -ansi_alias -ansi_args -fp_reorder -tune host"
		      if test "x$acx_maxopt_portable" = xno; then
			 CFLAGS="$CFLAGS -arch host"
		      fi
                fi;;

    hppa*-hpux*)  if test "$ac_compiler_gnu" != yes; then
                      CFLAGS="+Oall +Optrs_ansi -Wp,-H128000 +DSnative"
	  	      if test "x$acx_maxopt_portable" = xyes; then
			 CFLAGS="$CFLAGS +DAportable"
		      fi
                  fi;;

    *-aix*)
	if test "$base_CC" = cc -o "$base_CC" = xlc -o "$base_CC" = ccc; then
		if test "x$acx_maxopt_portable" = xno; then
                	xlc_opt="-qarch=auto -qtune=auto"
		else
                	xlc_opt="-qtune=auto"
		fi
                AX_CHECK_COMPILER_FLAGS($xlc_opt,
                        CFLAGS="-O3 -qansialias -w $xlc_opt",
                        [CFLAGS="-O3 -qansialias -w"
                echo "*******************************************************"
                echo "*  You seem to have AIX and the IBM compiler.  It is  *"
                echo "*  recommended for best performance that you use:     *"
                echo "*                                                     *"
                echo "*    CFLAGS=-O3 -qarch=xxx -qtune=xxx -qansialias -w  *"
                echo "*                      ^^^        ^^^                 *"
                echo "*  where xxx is pwr2, pwr3, 604, or whatever kind of  *"
                echo "*  CPU you have.  (Set the CFLAGS environment var.    *"
                echo "*  and re-run configure.)  For more info, man cc.     *"
                echo "*******************************************************"])
        fi;;
  esac

  else # using GCC

     # default optimization flags for gcc on all systems
     CFLAGS="-O3 -fomit-frame-pointer"

     # test for gcc-specific flags:
     #   -malign-double for x86 systems
     AX_CHECK_COMPILER_FLAGS(-malign-double, CFLAGS="$CFLAGS -malign-double")
     #  -fstrict-aliasing for gcc-2.95+
     AX_CHECK_COMPILER_FLAGS(-fstrict-aliasing,
	 CFLAGS="$CFLAGS -fstrict-aliasing")

     AX_GCC_ARCHFLAG($acx_maxopt_portable)

  fi # using GCC

  if test -z "$CFLAGS"; then
	echo ""
	echo "********************************************************"
        echo "* WARNING: Don't know the best CFLAGS for this system  *"
        echo "* Use  make CFLAGS=..., or edit the top level Makefile *"
	echo "* (otherwise, a default of CFLAGS=-O3 will be used)    *"
	echo "********************************************************"
	echo ""
        CFLAGS="-O3"
  fi

  AX_CHECK_COMPILER_FLAGS($CFLAGS, [], [
	echo ""
        echo "********************************************************"
        echo "* WARNING: The guessed CFLAGS don't seem to work with  *"
        echo "* your compiler.                                       *"
        echo "* Use  make CFLAGS=..., or edit the top level Makefile *"
        echo "********************************************************"
        echo ""
        CFLAGS=""
  ])

fi
])
