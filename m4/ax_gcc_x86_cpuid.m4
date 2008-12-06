dnl @synopsis AX_GCC_X86_CPUID(OP)
dnl @summary run x86 cpuid instruction OP using gcc inline assembler
dnl @category Misc
dnl
dnl On Pentium and later x86 processors, with gcc or a compiler that
dnl has a compatible syntax for inline assembly instructions, run
dnl a small program that executes the cpuid instruction with
dnl input OP.  This can be used to detect the CPU type.
dnl
dnl On output, the values of the eax, ebx, ecx, and edx registers
dnl are stored as hexadecimal strings as "eax:ebx:ecx:edx" in
dnl the cache variable ax_cv_gcc_x86_cpuid_OP.
dnl
dnl If the cpuid instruction fails (because you are running a cross-compiler,
dnl or because you are not using gcc, or because you are on a processor
dnl that doesn't have this instruction), ax_cv_gcc_x86_cpuid_OP is set
dnl to the string "unknown".
dnl
dnl This macro mainly exists to be used in AX_GCC_ARCHFLAG.
dnl
dnl @version 2008-12-06
dnl @license GPLWithACException
dnl @author Steven G. Johnson <stevenj@alum.mit.edu> and Matteo Frigo.
AC_DEFUN([AX_GCC_X86_CPUID],
[AC_REQUIRE([AC_PROG_CC])
AC_LANG_PUSH([C])
AC_CACHE_CHECK(for x86 cpuid $1 output, ax_cv_gcc_x86_cpuid_$1,
 [AC_RUN_IFELSE([AC_LANG_PROGRAM([#include <stdio.h>], [
     int op = $1, eax, ebx, ecx, edx;
     FILE *f;
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
     __asm__("push %%rbx\n\t"
             "cpuid\n\t"
             "pop %%rbx"
             : "=a" (eax), "=c" (ecx), "=d" (edx)
             : "a" (op));
     __asm__("push %%rbx\n\t"
             "cpuid\n\t"
             "mov %%rbx, %%rax\n\t"
             "pop %%rbx"
             : "=a" (ebx), "=c" (ecx), "=d" (edx)
             : "a" (op));
#else
     __asm__("push %%ebx\n\t"
             "cpuid\n\t"
             "pop %%ebx"
             : "=a" (eax), "=c" (ecx), "=d" (edx)
             : "a" (op));
     __asm__("push %%ebx\n\t"
             "cpuid\n\t"
             "mov %%ebx, %%eax\n\t"
             "pop %%ebx"
             : "=a" (ebx), "=c" (ecx), "=d" (edx)
             : "a" (op));
#endif
     f = fopen("conftest_cpuid", "w"); if (!f) return 1;
     fprintf(f, "%x:%x:%x:%x\n", eax, ebx, ecx, edx);
     fclose(f);
     return 0;
])], 
     [ax_cv_gcc_x86_cpuid_$1=`cat conftest_cpuid`; rm -f conftest_cpuid],
     [ax_cv_gcc_x86_cpuid_$1=unknown; rm -f conftest_cpuid],
     [ax_cv_gcc_x86_cpuid_$1=unknown])])
AC_LANG_POP([C])
])
