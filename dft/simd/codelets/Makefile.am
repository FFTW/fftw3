AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft	\
-I$(top_srcdir)/dft/simd -I$(top_srcdir)/simd
noinst_LTLIBRARIES = libdft_codelets_simd.la

N4 = n4simd_4.c n4simd_8.c n4simd_12.c n4simd_16.c n4simd_32.c	\
n4simd_64.c

T4 = t4simd_4.c t4simd_8.c t4simd_16.c t4simd_32.c t4simd_64.c

SIMD_CODELETS = $(N4) $(T4)

if HAVE_SIMD
ALL_CODELETS = $(SIMD_CODELETS)
else
ALL_CODELETS = 
endif

EXTRA_DIST = $(SIMD_CODELETS)
BUILT_SOURCES= $(ALL_CODELETS) $(CODLIST)

libdft_codelets_simd_la_SOURCES = $(BUILT_SOURCES)

SOLVTAB_NAME = X(solvtab_dft_simd)

# special rules for regenerating codelets.
include $(top_srcdir)/support/Makefile.codelets

if MAINTAINER_MODE
GFLAGS = -no-randomized-cse -simd -compact
n4simd_%.c:  $(CODELET_DEPS) $(GEN_NOTW)
	($(PRELUDE_COMMANDS); $(GEN_NOTW) $(GFLAGS) -n $* -name n4simd_$* -include "n4.h" -store-transpose -trivial-stores -with-ostride 1) | $(ADD_DATE) | $(INDENT) >$@

t4simd_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS); $(GEN_TWIDDLE) $(GFLAGS) -n $* -name t4simd_$* -include "t4.h" -trivial-stores) | $(ADD_DATE) | $(INDENT) >$@

endif # MAINTAINER_MODE
