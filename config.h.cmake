/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to compile in long-double precision. */
#cmakedefine BENCHFFT_LDOUBLE @BENCHFFT_LDOUBLE@

/* Define to compile in quad precision. */
#cmakedefine BENCHFFT_QUAD @BENCHFFT_QUAD@

/* Define to compile in single precision. */
#cmakedefine BENCHFFT_SINGLE @BENCHFFT_SINGLE@

/* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
   systems. This function is required for `alloca.c' support on those systems.
   */
#cmakedefine CRAY_STACKSEG_END @CRAY_STACKSEG_END@

/* Define to 1 if using `alloca.c'. */
#cmakedefine C_ALLOCA @C_ALLOCA@

/* Define to disable Fortran wrappers. */
#cmakedefine DISABLE_FORTRAN @DISABLE_FORTRAN@

/* Define to dummy `main' function (if any) required to link to the Fortran
   libraries. */
#cmakedefine F77_DUMMY_MAIN @F77_DUMMY_MAIN@

/* Define to a macro mangling the given C identifier (in lower and upper
   case), which must not contain underscores, for linking with Fortran. */
#cmakedefine F77_FUNC @F77_FUNC@

/* As F77_FUNC, but for C identifiers containing underscores. */
#cmakedefine F77_FUNC_ @F77_FUNC_@

/* Define if F77_FUNC and F77_FUNC_ are equivalent. */
#cmakedefine F77_FUNC_EQUIV @F77_FUNC_EQUIV@

/* Define if F77 and FC dummy `main' functions are identical. */
#cmakedefine FC_DUMMY_MAIN_EQ_F77 @FC_DUMMY_MAIN_EQ_F77@

/* C compiler name and flags */
#cmakedefine FFTW_CC "@FFTW_CC@"

/* Define to enable extra FFTW debugging code. */
#cmakedefine FFTW_DEBUG "@FFTW_DEBUG@"

/* Define to enable alignment debugging hacks. */
#cmakedefine FFTW_DEBUG_ALIGNMENT @FFTW_DEBUG_ALIGNMENT@

/* Define to enable debugging malloc. */
#cmakedefine FFTW_DEBUG_MALLOC @FFTW_DEBUG_MALLOC@

/* Define to enable the use of alloca(). */
#cmakedefine FFTW_ENABLE_ALLOCA @FFTW_ENABLE_ALLOCA@

/* Define to compile in long-double precision. */
#cmakedefine FFTW_LDOUBLE @FFTW_LDOUBLE@

/* Define to compile in quad precision. */
#cmakedefine FFTW_QUAD @FFTW_QUAD@

/* Define to enable pseudorandom estimate planning for debugging. */
#cmakedefine FFTW_RANDOM_ESTIMATOR @FFTW_RANDOM_ESTIMATOR@

/* Define to compile in single precision. */
#cmakedefine FFTW_SINGLE @FFTW_SINGLE@

/* Define to 1 if you have the `abort' function. */
#cmakedefine HAVE_ABORT @HAVE_ABORT@

/* Define to 1 if you have `alloca', as a function or macro. */
#cmakedefine HAVE_ALLOCA @HAVE_ALLOCA@

/* Define to 1 if you have <alloca.h> and it should be used (not on Ultrix).
   */
#cmakedefine HAVE_ALLOCA_H @HAVE_ALLOCA_H@

/* Define to enable Altivec optimizations. */
#cmakedefine HAVE_ALTIVEC @HAVE_ALTIVEC@

/* Define to 1 if you have the <altivec.h> header file. */
#cmakedefine HAVE_ALTIVEC_H @HAVE_ALTIVEC_H@

/* Define to enable AVX optimizations. */
#cmakedefine HAVE_AVX @HAVE_AVX@

/* Define to 1 if you have the `BSDgettimeofday' function. */
#cmakedefine HAVE_BSDGETTIMEOFDAY @HAVE_BSDGETTIMEOFDAY@

/* Define to 1 if you have the `clock_gettime' function. */
#cmakedefine HAVE_CLOCK_GETTIME @HAVE_CLOCK_GETTIME@

/* Define to 1 if you have the `cosl' function. */
#cmakedefine HAVE_COSL @HAVE_COSL@

/* Define to 1 if you have the <c_asm.h> header file. */
#cmakedefine HAVE_C_ASM_H @HAVE_C_ASM_H@

/* Define to 1 if you have the declaration of `cosl', and to 0 if you don't.
   */
#cmakedefine HAVE_DECL_COSL @HAVE_DECL_COSL@

/* Define to 1 if you have the declaration of `cosq', and to 0 if you don't.
   */
#cmakedefine HAVE_DECL_COSQ @HAVE_DECL_COSQ@

/* Define to 1 if you have the declaration of `drand48', and to 0 if you
   don't. */
#cmakedefine HAVE_DECL_DRAND48 @HAVE_DECL_DRAND48@

/* Define to 1 if you have the declaration of `memalign', and to 0 if you
   don't. */
#cmakedefine HAVE_DECL_MEMALIGN @HAVE_DECL_MEMALIGN@

/* Define to 1 if you have the declaration of `posix_memalign', and to 0 if
   you don't. */
#cmakedefine HAVE_DECL_POSIX_MEMALIGN @HAVE_DECL_POSIX_MEMALIGN@

/* Define to 1 if you have the declaration of `sinl', and to 0 if you don't.
   */
#cmakedefine HAVE_DECL_SINL @HAVE_DECL_SINL@

/* Define to 1 if you have the declaration of `sinq', and to 0 if you don't.
   */
#cmakedefine HAVE_DECL_SINQ @HAVE_DECL_SINQ@

/* Define to 1 if you have the declaration of `srand48', and to 0 if you
   don't. */
#cmakedefine HAVE_DECL_SRAND48 @HAVE_DECL_SRAND48@

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H @HAVE_DLFCN_H@

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
#cmakedefine HAVE_DOPRNT @HAVE_DOPRNT@

/* Define to 1 if you have the `drand48' function. */
#cmakedefine HAVE_DRAND48 @HAVE_DRAND48@

/* Define if you have a machine with fused multiply-add */
#cmakedefine HAVE_FMA @HAVE_FMA@

/* Define to 1 if you have the `gethrtime' function. */
#cmakedefine HAVE_GETHRTIME @HAVE_GETHRTIME@

/* Define to 1 if you have the `gettimeofday' function. */
#cmakedefine HAVE_GETTIMEOFDAY @HAVE_GETTIMEOFDAY@

/* Define to 1 if hrtime_t is defined in <sys/time.h> */
#cmakedefine HAVE_HRTIME_T @HAVE_HRTIME_T@

/* Define to 1 if you have the <intrinsics.h> header file. */
#cmakedefine HAVE_INTRINSICS_H @HAVE_INTRINSICS_H@

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H @HAVE_INTTYPES_H@

/* Define if the isnan() function/macro is available. */
#cmakedefine HAVE_ISNAN @HAVE_ISNAN@

/* Define to 1 if you have the <libintl.h> header file. */
#cmakedefine HAVE_LIBINTL_H @HAVE_LIBINTL_H@

/* Define to 1 if you have the `m' library (-lm). */
#cmakedefine HAVE_LIBM @HAVE_LIBM@

/* Define to 1 if you have the `quadmath' library (-lquadmath). */
#cmakedefine HAVE_LIBQUADMATH @HAVE_LIBQUADMATH@

/* Define to 1 if you have the <limits.h> header file. */
#cmakedefine HAVE_LIMITS_H @HAVE_LIMITS_H@

/* Define to 1 if the compiler supports `long double' */
#cmakedefine HAVE_LONG_DOUBLE @HAVE_LONG_DOUBLE@

/* Define to 1 if you have the `mach_absolute_time' function. */
#cmakedefine HAVE_MACH_ABSOLUTE_TIME @HAVE_MACH_ABSOLUTE_TIME@

/* Define to 1 if you have the <mach/mach_time.h> header file. */
#cmakedefine HAVE_MACH_MACH_TIME_H @HAVE_MACH_MACH_TIME_H@

/* Define to 1 if you have the <malloc.h> header file. */
#cmakedefine HAVE_MALLOC_H @HAVE_MALLOC_H@

/* Define to 1 if you have the `memalign' function. */
#cmakedefine HAVE_MEMALIGN @HAVE_MEMALIGN@

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H @HAVE_MEMORY_H@

/* Define to 1 if you have the `memset' function. */
#cmakedefine HAVE_MEMSET @HAVE_MEMSET@

/* Define to enable use of MIPS ZBus cycle-counter. */
#cmakedefine HAVE_MIPS_ZBUS_TIMER @HAVE_MIPS_ZBUS_TIMER@

/* Define if you have the MPI library. */
#cmakedefine HAVE_MPI @HAVE_MPI@

/* Define to enable ARM NEON optimizations. */
#cmakedefine HAVE_NEON @HAVE_NEON@

/* Define if OpenMP is enabled */
#cmakedefine HAVE_OPENMP @HAVE_OPENMP@

/* Define to 1 if you have the `posix_memalign' function. */
#cmakedefine HAVE_POSIX_MEMALIGN @HAVE_POSIX_MEMALIGN@

/* Define if you have POSIX threads libraries and header files. */
#cmakedefine HAVE_PTHREAD @HAVE_PTHREAD@

/* Define to 1 if you have the `read_real_time' function. */
#cmakedefine HAVE_READ_REAL_TIME @HAVE_READ_REAL_TIME@

/* Define to 1 if you have the `sinl' function. */
#cmakedefine HAVE_SINL @HAVE_SINL@

/* Define to 1 if you have the `snprintf' function. */
#cmakedefine HAVE_SNPRINTF @HAVE_SNPRINTF@

/* Define to 1 if you have the `sqrt' function. */
#cmakedefine HAVE_SQRT @HAVE_SQRT@

/* Define to enable SSE/SSE2 optimizations. */
#cmakedefine HAVE_SSE2 @HAVE_SSE2@

/* Define to 1 if you have the <stddef.h> header file. */
#cmakedefine HAVE_STDDEF_H @HAVE_STDDEF_H@

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H @HAVE_STDINT_H@

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H @HAVE_STDLIB_H@

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H @HAVE_STRINGS_H@

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H @HAVE_STRING_H@

/* Define to 1 if you have the `sysctl' function. */
#cmakedefine HAVE_SYSCTL @HAVE_SYSCTL@

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H @HAVE_SYS_STAT_H@

/* Define to 1 if you have the <sys/sysctl.h> header file. */
#cmakedefine HAVE_SYS_SYSCTL_H @HAVE_SYS_SYSCTL_H@

/* Define to 1 if you have the <sys/time.h> header file. */
#cmakedefine HAVE_SYS_TIME_H @HAVE_SYS_TIME_H@

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H @HAVE_SYS_TYPES_H@

/* Define to 1 if you have the `tanl' function. */
#cmakedefine HAVE_TANL @HAVE_TANL@

/* Define if we have a threads library. */
#cmakedefine HAVE_THREADS @HAVE_THREADS@

/* Define to 1 if you have the `time_base_to_time' function. */
#cmakedefine HAVE_TIME_BASE_TO_TIME @HAVE_TIME_BASE_TO_TIME@

/* Define to 1 if the system has the type `uintptr_t'. */
#cmakedefine HAVE_UINTPTR_T @HAVE_UINTPTR_T@

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H @HAVE_UNISTD_H@

/* Define to 1 if you have the `vprintf' function. */
#cmakedefine HAVE_VPRINTF @HAVE_VPRINTF@

/* Define to 1 if you have the `_mm_free' function. */
#cmakedefine HAVE__MM_FREE @HAVE__MM_FREE@

/* Define to 1 if you have the `_mm_malloc' function. */
#cmakedefine HAVE__MM_MALLOC @HAVE__MM_MALLOC@

/* Define if you have the UNICOS _rtc() intrinsic. */
#cmakedefine HAVE__RTC @HAVE__RTC@

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#cmakedefine LT_OBJDIR @LT_OBJDIR@

/* Name of package */
#cmakedefine PACKAGE "@PACKAGE@"

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine PACKAGE_BUGREPORT "@PACKAGE_BUGREPORT@"

/* Define to the full name of this package. */
#cmakedefine PACKAGE_NAME "@PACKAGE_NAME@"

/* Define to the full name and version of this package. */
#cmakedefine PACKAGE_STRING "@PACKAGE_STRING@"

/* Define to the one symbol short name of this package. */
#cmakedefine PACKAGE_TARNAME "@PACKAGE_TARNAME@"

/* Define to the home page for this package. */
#cmakedefine PACKAGE_URL "@PACKAGE_URL@"

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION "@PACKAGE_VERSION@"

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
#cmakedefine PTHREAD_CREATE_JOINABLE @PTHREAD_CREATE_JOINABLE@

/* The size of `double', as computed by sizeof. */
#cmakedefine SIZEOF_DOUBLE @SIZEOF_DOUBLE@

/* The size of `fftw_r2r_kind', as computed by sizeof. */
#cmakedefine SIZEOF_FFTW_R2R_KIND @SIZEOF_FFTW_R2R_KIND@

/* The size of `float', as computed by sizeof. */
#cmakedefine SIZEOF_FLOAT @SIZEOF_FLOAT@

/* The size of `int', as computed by sizeof. */
#cmakedefine SIZEOF_INT @SIZEOF_INT@

/* The size of `long', as computed by sizeof. */
#cmakedefine SIZEOF_LONG @SIZEOF_LONG@

/* The size of `long long', as computed by sizeof. */
#cmakedefine SIZEOF_LONG_LONG @SIZEOF_LONG_LONG@

/* The size of `MPI_Fint', as computed by sizeof. */
#cmakedefine SIZEOF_MPI_FINT @SIZEOF_MPI_FINT@

/* The size of `ptrdiff_t', as computed by sizeof. */
#cmakedefine SIZEOF_PTRDIFF_T @SIZEOF_PTRDIFF_T@

/* The size of `size_t', as computed by sizeof. */
#cmakedefine SIZEOF_SIZE_T @SIZEOF_SIZE_T@

/* The size of `unsigned int', as computed by sizeof. */
#cmakedefine SIZEOF_UNSIGNED_INT @SIZEOF_UNSIGNED_INT@

/* The size of `unsigned long', as computed by sizeof. */
#cmakedefine SIZEOF_UNSIGNED_LONG @SIZEOF_UNSIGNED_LONG@

/* The size of `unsigned long long', as computed by sizeof. */
#cmakedefine SIZEOF_UNSIGNED_LONG_LONG @SIZEOF_UNSIGNED_LONG_LONG@

/* The size of `void *', as computed by sizeof. */
#cmakedefine SIZEOF_VOID_P @SIZEOF_VOID_P@

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at runtime.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown */
#cmakedefine STACK_DIRECTION @STACK_DIRECTION@

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS @STDC_HEADERS@

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#cmakedefine TIME_WITH_SYS_TIME @TIME_WITH_SYS_TIME@

/* Define if we have and are using POSIX threads. */
#cmakedefine USING_POSIX_THREADS @USING_POSIX_THREADS@

/* Version number of package */
#cmakedefine VERSION "@VERSION@"

/* Use common Windows Fortran mangling styles for the Fortran interfaces. */
#cmakedefine WINDOWS_F77_MANGLING @WINDOWS_F77_MANGLING@

/* Include g77-compatible wrappers in addition to any other Fortran wrappers.
   */
#cmakedefine WITH_G77_WRAPPERS @WITH_G77_WRAPPERS@

/* Use our own aligned malloc routine; mainly helpful for Windows systems
   lacking aligned allocation system-library routines. */
#cmakedefine WITH_OUR_MALLOC @WITH_OUR_MALLOC@

/* Use low-precision timers, making planner very slow */
#cmakedefine WITH_SLOW_TIMER @WITH_SLOW_TIMER@

/* Define to empty if `const' does not conform to ANSI C. */
#cmakedefine const @const@

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#cmakedefine inline @inline@
#endif

/* Define to `unsigned int' if <sys/types.h> does not define. */
#cmakedefine size_t @size_t@
