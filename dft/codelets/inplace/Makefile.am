AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft
noinst_LTLIBRARIES = libdft_inplace.la

F1_CODELETS = Kf1_2.c Kf1_3.c Kf1_4.c Kf1_5.c Kf1_6.c Kf1_7.c Kf1_8.c	\
Kf1_9.c Kf1_10.c Kf1_16.c Kf1_32.c Kf1_64.c

if RESEARCH_MODE
F2_CODELETS = Kf2_4.c Kf2_8.c Kf2_16.c Kf2_32.c Kf2_64.c
Q1_CODELETS = Kq1_2.c Kq1_4.c Kq1_8.c
endif

ALL_CODELETS = $(F1_CODELETS) $(F2_CODELETS) $(Q1_CODELETS) 

BUILT_SOURCES=$(ALL_CODELETS) $(CODLIST)

libdft_inplace_la_SOURCES = $(BUILT_SOURCES)

SOLVTAB_NAME = X(solvtab_dft_inplace)

# special rules for regenerating codelets.
include $(top_srcdir)/support/Makefile.codelets

if MAINTAINER_MODE

Kf1_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS); $(GEN_TWIDDLE) $(FLAGS_F1) -dif -n $* -name Kf1_$*) | $(ADD_DATE) | $(INDENT) >$@

Kf2_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS); $(GEN_TWIDDLE) $(FLAGS_F2) -dif -n $* -name Kf2_$*) | $(ADD_DATE) | $(INDENT) >$@

Kq1_%.c:  $(CODELET_DEPS) $(GEN_TWIDSQ)
	($(PRELUDE_COMMANDS); $(GEN_TWIDSQ) $(FLAGS_Q1) -dif -n $* -name Kq1_$*) | $(ADD_DATE) | $(INDENT) >$@

Kq2_%.c:  $(CODELET_DEPS) $(GEN_TWIDSQ)
	($(PRELUDE_COMMANDS); $(GEN_TWIDSQ) $(FLAGS_Q2) -dif -n $* -name Kq2_$*) | $(ADD_DATE) | $(INDENT) >$@

endif # MAINTAINER_MODE