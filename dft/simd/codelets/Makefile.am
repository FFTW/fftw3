AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft	\
-I$(top_srcdir)/dft/simd -I$(top_srcdir)/simd
noinst_LTLIBRARIES = libdft_codelets_simd.la

N2F = n2fsimd_2.c n2fsimd_4.c n2fsimd_8.c n2fsimd_12.c n2fsimd_16.c	\
n2fsimd_32.c n2fsimd_64.c

N3F = n3fsimd_2.c n3fsimd_4.c n3fsimd_8.c n3fsimd_12.c n3fsimd_16.c	\
n3fsimd_32.c n3fsimd_64.c

N4 = n4simd_4.c n4simd_8.c n4simd_12.c n4simd_16.c n4simd_32.c	\
n4simd_64.c

T2F = t2fsimd_4.c t2fsimd_8.c t2fsimd_16.c t2fsimd_32.c t2fsimd_64.c

T3F = t3fsimd_4.c t3fsimd_8.c t3fsimd_16.c t3fsimd_32.c t3fsimd_64.c

T4 = t4simd_4.c t4simd_8.c t4simd_16.c t4simd_32.c t4simd_64.c

SIMD_CODELETS = $(N2F) $(N3F) $(N4) $(T2F) $(T3F) $(T4)

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
GFLAGS = -no-randomized-cse -simd -compact -trivial-stores

n2fsimd_%.c:  $(CODELET_DEPS) $(GEN_NOTW)
	($(PRELUDE_COMMANDS); $(GEN_NOTW) $(GFLAGS) -n $* -name n2fsimd_$* -include "n2f.h" -ldri2 -with-ivstride 2 -stri) | $(ADD_DATE) | $(INDENT) >$@

n3fsimd_%.c:  $(CODELET_DEPS) $(GEN_NOTW)
	($(PRELUDE_COMMANDS); $(GEN_NOTW) $(GFLAGS) -n $* -name n3fsimd_$* -include "n3f.h" -ldri2 -stri2 -with-ivstride 2 -with-ovstride 2) | $(ADD_DATE) | $(INDENT) >$@

n4simd_%.c:  $(CODELET_DEPS) $(GEN_NOTW)
	($(PRELUDE_COMMANDS); $(GEN_NOTW) $(GFLAGS) -n $* -name n4simd_$* -include "n4.h" -sttr -with-ostride 1 -with-ivstride 1) | $(ADD_DATE) | $(INDENT) >$@

t2fsimd_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS); $(GEN_TWIDDLE) $(GFLAGS) -n $* -name t2fsimd_$* -ldri -stri -include "t2f.h") | $(ADD_DATE) | $(INDENT) >$@

t3fsimd_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS); $(GEN_TWIDDLE) $(GFLAGS) -n $* -name t3fsimd_$* -ldri2 -stri2 -with-dist 2 -include "t3f.h") | $(ADD_DATE) | $(INDENT) >$@

t4simd_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS); $(GEN_TWIDDLE) $(GFLAGS) -n $* -name t4simd_$* -with-dist 1 -include "t4.h") | $(ADD_DATE) | $(INDENT) >$@

endif # MAINTAINER_MODE
