AC_DEFUN([OCAML_INIT_PATHS],
[
eval "ocaml_prefix=$exec_prefix"
if test "x$ocaml_prefix" = xNONE; then
	eval "ocaml_prefix=$prefix"
	test "x$ocaml_prefix" = xNONE && ocaml_prefix=$ac_default_prefix
fi

OCAML_BINDIR="${ocaml_prefix}/bin"
OCAML_LIBDIR="${ocaml_prefix}/lib"
OCAML_TARGET_BINDIR="${ocaml_prefix}/bin"
INSTALLED_OCAMLC=$OCAML_BINDIR/ocamlc
AC_SUBST(INSTALLED_OCAMLC)
TARGET_OCAMLLIB=$OCAML_LIBDIR/$PACKAGE
AC_SUBST(TARGET_OCAMLLIB)
TARGET_OCAMLBIN=$OCAML_TARGET_BINDIR
AC_SUBST(TARGET_OCAMLBIN)
TARGET_OCAMLRUN=$TARGET_OCAMLBIN/ocamlrun
AC_SUBST(TARGET_OCAMLRUN)
])

AC_DEFUN([OCAML_CHECK_TOOLS],
[
AC_ARG_ENABLE(bootstrap,  [  --enable-bootstrap      use the bootstrap ocaml compiler], enable_bootstrap=$enableval, enable_bootstrap=no)
ocaml_srcdir='${top_srcdir}'/$1
ocaml_builddir='${top_builddir}'/$1
AC_SUBST(ocaml_srcdir)
AC_SUBST(ocaml_builddir)

byterun_srcdir=$ocaml_srcdir/target/byterun
AC_SUBST(byterun_srcdir)
byterun_builddir=$ocaml_builddir/target/byterun
AC_SUBST(byterun_builddir)
asmrun_srcdir=$ocaml_srcdir/target/asmrun
AC_SUBST(asmrun_srcdir)
asmrun_builddir=$ocaml_builddir/target/asmrun
AC_SUBST(asmrun_builddir)
compiler_srcdir=$ocaml_srcdir/compiler
AC_SUBST(compiler_srcdir)
compiler_builddir=$ocaml_builddir/compiler
AC_SUBST(compiler_builddir)

ocamlrun=$byterun_builddir/ocamlrun
ocamlboot=$ocaml_srcdir/boot
BOOT_OCAMLC="$ocamlrun $ocamlboot/ocamlc -with-stdlib $ocaml_builddir/stdlib"
BOOT_OCAMLLEX="$ocamlrun $ocamlboot/ocamllex"
BOOT_OCAMLDEP="$ocamlrun $compiler_builddir/ocamldep"
BOOT_OCAMLYACC="$ocaml_builddir/yacc/ocamlyacc"
BOOT_OCAMLRUN=$ocamlrun
OCAMLC="\${OCAMLRUN} $compiler_builddir/ocamlc -with-stdlib $ocaml_builddir/stdlib"
OCAMLOPT="\${OCAMLRUN} $compiler_builddir/ocamlopt -with-stdlib $ocaml_builddir/stdlib"
if test "$enable_bootstrap" = "yes"; then
	OCAMLC_FOR_COMPILER=$BOOT_OCAMLC
	OCAMLC_FOR_STDLIB=$BOOT_OCAMLC
	OCAMLLEX=$BOOT_OCAMLLEX
	OCAMLDEP=$BOOT_OCAMLDEP
	OCAMLYACC=$BOOT_OCAMLYACC
	OCAMLRUN=$BOOT_OCAMLRUN
else
	OCAMLC_FOR_STDLIB=$OCAMLC
fi
OCAMLOPT_FOR_STDLIB=$OCAMLOPT
AC_SUBST(OCAMLC)
AC_SUBST(OCAMLOPT)
AC_CHECK_PROG(OCAMLC_FOR_STDLIB, ocamlc, ocamlc, $BOOT_OCAMLC)
AC_CHECK_PROG(OCAMLC_FOR_COMPILER, ocamlc, ocamlc, $BOOT_OCAMLC)
AC_CHECK_PROG(OCAMLOPT_FOR_STDLIB, ocamlopt, ocamlopt, $BOOT_OCAMLOPT)
AC_CHECK_PROG(OCAMLYACC, ocamlyacc, ocamlyacc, $BOOT_OCAMLYACC)
AC_CHECK_PROG(OCAMLLEX, ocamllex, ocamllex, $BOOT_OCAMLLEX)
AC_CHECK_PROG(OCAMLDEP, ocamldep, ocamldep, $BOOT_OCAMLDEP)
AC_CHECK_PROG(OCAMLRUN, ocamlrun, ocamlrun, $BOOT_OCAMLRUN)

AC_SUBST(OCAMLCFLAGS)
AC_SUBST(OCAMLOPTCFLAGS)
AC_SUBST(OCAMLLDFLAGS)

]
)
