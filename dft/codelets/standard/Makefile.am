AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft	\
-I$(top_srcdir)/dft/codelets
CFLAGS = @CFLAGS@ @CODELET_OPTIM@
noinst_LTLIBRARIES = libdft_standard.la

N1 = n1_2.c n1_3.c n1_4.c n1_5.c n1_6.c n1_7.c n1_8.c n1_9.c n1_10.c	\
n1_11.c n1_12.c n1_13.c n1_14.c n1_15.c n1_16.c n1_32.c n1_64.c

T1 = t1_2.c t1_3.c t1_4.c t1_5.c t1_6.c t1_7.c t1_8.c t1_9.c	\
t1_10.c t1_12.c t1_15.c t1_16.c t1_32.c t1_64.c

T2 = t2_4.c t2_8.c t2_16.c t2_32.c t2_64.c

ALL_CODELETS = $(N1) $(T1) $(T2)

BUILT_SOURCES= $(ALL_CODELETS) $(CODLIST)

libdft_standard_la_SOURCES = $(BUILT_SOURCES)

SOLVTAB_NAME = X(solvtab_dft_standard)

# special rules for regenerating codelets.
include $(top_srcdir)/support/Makefile.codelets

if MAINTAINER_MODE
FLAGS_N1=$(DFT_FLAGS_COMMON)
FLAGS_T1=$(DFT_FLAGS_COMMON)
FLAGS_T2=$(DFT_FLAGS_COMMON) -twiddle-log3

n1_%.c:  $(CODELET_DEPS) $(GEN_NOTW)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW) $(FLAGS_N1) -n $* -name n1_$* -include "n.h") | $(ADD_DATE) | $(INDENT) >$@

t1_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS_DFT); $(GEN_TWIDDLE) $(FLAGS_T1) -n $* -name t1_$* -include "t.h") | $(ADD_DATE) | $(INDENT) >$@

t2_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS_DFT); $(GEN_TWIDDLE) $(FLAGS_T2) -n $* -name t2_$* -include "t.h") | $(ADD_DATE) | $(INDENT) >$@

endif # MAINTAINER_MODE