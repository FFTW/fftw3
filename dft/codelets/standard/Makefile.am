AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft
noinst_LTLIBRARIES = libdft_standard.la

N1_CODELETS = Kn1_2.c Kn1_3.c Kn1_4.c Kn1_5.c Kn1_6.c Kn1_7.c Kn1_8.c	\
Kn1_9.c Kn1_10.c Kn1_11.c Kn1_12.c Kn1_13.c Kn1_14.c Kn1_15.c Kn1_16.c	\
Kn1_32.c Kn1_64.c

T1_CODELETS = Kt1_2.c Kt1_3.c Kt1_4.c Kt1_5.c Kt1_6.c Kt1_7.c Kt1_8.c	\
Kt1_9.c Kt1_10.c Kt1_16.c Kt1_32.c Kt1_64.c

if RESEARCH_MODE
T2_CODELETS = Kt2_4.c Kt2_8.c Kt2_16.c Kt2_32.c Kt2_64.c
endif

ALL_CODELETS = $(N1_CODELETS) $(T1_CODELETS) $(T2_CODELETS)

BUILT_SOURCES= $(ALL_CODELETS) $(CODLIST)

libdft_standard_la_SOURCES = $(BUILT_SOURCES)

SOLVTAB_NAME = X(solvtab_dft_standard)

# special rules for regenerating codelets.
include $(top_srcdir)/support/Makefile.codelets

if MAINTAINER_MODE

Kn1_%.c:  $(CODELET_DEPS) $(GEN_NOTW)
	($(PRELUDE_COMMANDS); $(GEN_NOTW) $(FLAGS_N1) -n $* -name Kn1_$*) | $(ADD_DATE) | $(INDENT) >$@

Kt1_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS); $(GEN_TWIDDLE) $(FLAGS_T1) -n $* -name Kt1_$*) | $(ADD_DATE) | $(INDENT) >$@

Kt2_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS); $(GEN_TWIDDLE) $(FLAGS_T2) -n $* -name Kt2_$*) | $(ADD_DATE) | $(INDENT) >$@

endif # MAINTAINER_MODE