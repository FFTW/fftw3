AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft	\
-I$(top_srcdir)/dft/codelets
noinst_LTLIBRARIES = libdft_inplace.la

F1_CODELETS = f1_2.c f1_3.c f1_4.c f1_5.c f1_6.c f1_7.c f1_8.c f1_9.c	\
f1_10.c f1_16.c f1_32.c f1_64.c

F2_CODELETS = f2_4.c f2_8.c f2_16.c f2_32.c f2_64.c

Q1_CODELETS = q1_2.c q1_4.c q1_8.c

ALL_CODELETS = $(F1_CODELETS) $(F2_CODELETS) $(Q1_CODELETS) 

BUILT_SOURCES=$(ALL_CODELETS) $(CODLIST)

libdft_inplace_la_SOURCES = $(BUILT_SOURCES)

SOLVTAB_NAME = X(solvtab_dft_inplace)

# special rules for regenerating codelets.
include $(top_srcdir)/support/Makefile.codelets

if MAINTAINER_MODE

f1_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS); $(GEN_TWIDDLE) $(FLAGS_F1) -dif -n $* -name f1_$* -include "f.h") | $(ADD_DATE) | $(INDENT) >$@

f2_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS); $(GEN_TWIDDLE) $(FLAGS_F2) -dif -n $* -name f2_$* -include "f.h") | $(ADD_DATE) | $(INDENT) >$@

q1_%.c:  $(CODELET_DEPS) $(GEN_TWIDSQ)
	($(PRELUDE_COMMANDS); $(GEN_TWIDSQ) $(FLAGS_Q1) -dif -n $* -name q1_$* -include "q.h") | $(ADD_DATE) | $(INDENT) >$@

q2_%.c:  $(CODELET_DEPS) $(GEN_TWIDSQ)
	($(PRELUDE_COMMANDS); $(GEN_TWIDSQ) $(FLAGS_Q2) -dif -n $* -name q2_$* -include "q.h") | $(ADD_DATE) | $(INDENT) >$@

endif # MAINTAINER_MODE