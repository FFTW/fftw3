AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft	\
-I$(top_srcdir)/dft/simd -I$(top_srcdir)/simd
CFLAGS = @CFLAGS@ @CODELET_OPTIM@
noinst_LTLIBRARIES = libdft_codelets_simd.la

N1F = n1fsimd_2.c n1fsimd_3.c n1fsimd_4.c n1fsimd_5.c n1fsimd_6.c	\
n1fsimd_7.c n1fsimd_8.c n1fsimd_9.c n1fsimd_10.c n1fsimd_11.c		\
n1fsimd_12.c n1fsimd_13.c n1fsimd_14.c n1fsimd_15.c n1fsimd_16.c	\
n1fsimd_32.c n1fsimd_64.c

N1B = n1bsimd_2.c n1bsimd_3.c n1bsimd_4.c n1bsimd_5.c n1bsimd_6.c	\
n1bsimd_7.c n1bsimd_8.c n1bsimd_9.c n1bsimd_10.c n1bsimd_11.c		\
n1bsimd_12.c n1bsimd_13.c n1bsimd_14.c n1bsimd_15.c n1bsimd_16.c	\
n1bsimd_32.c n1bsimd_64.c

T1F = t1fsimd_2.c t1fsimd_3.c t1fsimd_4.c t1fsimd_5.c t1fsimd_6.c	\
t1fsimd_7.c t1fsimd_8.c t1fsimd_9.c t1fsimd_10.c t1fsimd_16.c		\
t1fsimd_32.c t1fsimd_64.c

T1B = t1bsimd_2.c t1bsimd_3.c t1bsimd_4.c t1bsimd_5.c t1bsimd_6.c	\
t1bsimd_7.c t1bsimd_8.c t1bsimd_9.c t1bsimd_10.c t1bsimd_16.c		\
t1bsimd_32.c t1bsimd_64.c

SIMD_CODELETS = $(N1F) $(N1B) $(T1F) $(T1B)

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
GFLAGS = -simd -trivial-stores $(FLAGS_COMMON)

n1fsimd_%.c:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS); $(GEN_NOTW_C) $(GFLAGS) -n $* -name n1fsimd_$* -include "n1f.h") | $(ADD_DATE) | $(INDENT) >$@

n1bsimd_%.c:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS); $(GEN_NOTW_C) $(GFLAGS) -sign 1 -n $* -name n1bsimd_$* -include "n1b.h") | $(ADD_DATE) | $(INDENT) >$@

t1fsimd_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE_C)
	($(PRELUDE_COMMANDS); $(GEN_TWIDDLE_C) $(GFLAGS) -n $* -name t1fsimd_$* -include "t1f.h") | $(ADD_DATE) | $(INDENT) >$@

t1bsimd_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE_C)
	($(PRELUDE_COMMANDS); $(GEN_TWIDDLE_C) $(GFLAGS) -n $* -name t1bsimd_$* -include "t1b.h" -sign 1) | $(ADD_DATE) | $(INDENT) >$@

endif # MAINTAINER_MODE
