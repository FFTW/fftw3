AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft	\
-I$(top_srcdir)/dft/simd -I$(top_srcdir)/simd
CFLAGS = @CFLAGS@ @CODELET_OPTIM@
noinst_LTLIBRARIES = libdft_codelets_simd.la

N1F = n1fv_2.c n1fv_3.c n1fv_4.c n1fv_5.c n1fv_6.c n1fv_7.c n1fv_8.c	\
n1fv_9.c n1fv_10.c n1fv_11.c n1fv_12.c n1fv_13.c n1fv_14.c n1fv_15.c	\
n1fv_16.c n1fv_32.c n1fv_64.c

N1B = n1bv_2.c n1bv_3.c n1bv_4.c n1bv_5.c n1bv_6.c n1bv_7.c n1bv_8.c	\
n1bv_9.c n1bv_10.c n1bv_11.c n1bv_12.c n1bv_13.c n1bv_14.c n1bv_15.c	\
n1bv_16.c n1bv_32.c n1bv_64.c

T1F = t1fv_2.c t1fv_3.c t1fv_4.c t1fv_5.c t1fv_6.c t1fv_7.c t1fv_8.c	\
t1fv_9.c t1fv_10.c t1fv_12.c t1fv_15.c t1fv_16.c t1fv_32.c t1fv_64.c

T1B = t1bv_2.c t1bv_3.c t1bv_4.c t1bv_5.c t1bv_6.c t1bv_7.c t1bv_8.c	\
t1bv_9.c t1bv_10.c t1bv_12.c t1bv_15.c t1bv_16.c t1bv_32.c t1bv_64.c

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

n1fv_%.c:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS); $(GEN_NOTW_C) $(GFLAGS) -n $* -name n1fv_$* -include "n1f.h") | $(ADD_DATE) | $(INDENT) >$@

n1bv_%.c:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS); $(GEN_NOTW_C) $(GFLAGS) -sign 1 -n $* -name n1bv_$* -include "n1b.h") | $(ADD_DATE) | $(INDENT) >$@

t1fv_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE_C)
	($(PRELUDE_COMMANDS); $(GEN_TWIDDLE_C) $(GFLAGS) -n $* -name t1fv_$* -include "t1f.h") | $(ADD_DATE) | $(INDENT) >$@

t1bv_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE_C)
	($(PRELUDE_COMMANDS); $(GEN_TWIDDLE_C) $(GFLAGS) -n $* -name t1bv_$* -include "t1b.h" -sign 1) | $(ADD_DATE) | $(INDENT) >$@

endif # MAINTAINER_MODE
