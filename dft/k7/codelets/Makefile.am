AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft
noinst_LTLIBRARIES = libdft_codelets_k7.la

# handle automake-1.5
AM_ASFLAGS = $(DEFAULT_INCLUDES)
# handle automake-1.6
AM_CCASFLAGS = $(DEFAULT_INCLUDES)

N1_CODELETS = Kn1k7_2.S Kn1k7_3.S Kn1k7_4.S Kn1k7_5.S Kn1k7_6.S		\
Kn1k7_7.S Kn1k7_8.S Kn1k7_9.S Kn1k7_10.S Kn1k7_11.S Kn1k7_12.S		\
Kn1k7_13.S Kn1k7_14.S Kn1k7_15.S Kn1k7_16.S Kn1k7_32.S Kn1k7_64.S	\
Kn1k7_128.S

N1I_CODELETS = Kn1k7i_2.S Kn1k7i_3.S Kn1k7i_4.S Kn1k7i_5.S Kn1k7i_6.S	\
Kn1k7i_7.S Kn1k7i_8.S Kn1k7i_9.S Kn1k7i_10.S Kn1k7i_11.S Kn1k7i_12.S	\
Kn1k7i_13.S Kn1k7i_14.S Kn1k7i_15.S Kn1k7i_16.S Kn1k7i_32.S		\
Kn1k7i_64.S Kn1k7i_128.S

T1_CODELETS = Kt1k7_2.S Kt1k7_3.S Kt1k7_4.S Kt1k7_5.S Kt1k7_6.S	\
Kt1k7_7.S Kt1k7_8.S Kt1k7_9.S Kt1k7_10.S Kt1k7_16.S Kt1k7_32.S	\
Kt1k7_64.S

T1I_CODELETS = Kt1k7i_2.S Kt1k7i_3.S Kt1k7i_4.S Kt1k7i_5.S Kt1k7i_6.S	\
Kt1k7i_7.S Kt1k7i_8.S Kt1k7i_9.S Kt1k7i_10.S Kt1k7i_16.S Kt1k7i_32.S	\
Kt1k7i_64.S

F1_CODELETS = Kf1k7_2.S Kf1k7_4.S Kf1k7_8.S Kf1k7_16.S Kf1k7_32.S	\
Kf1k7_64.S

F1I_CODELETS = Kf1k7i_2.S Kf1k7i_4.S Kf1k7i_8.S Kf1k7i_16.S	\
Kf1k7i_32.S Kf1k7i_64.S

K7_CODELETS = $(N1_CODELETS) $(N1I_CODELETS) $(T1_CODELETS)	\
$(T1I_CODELETS) $(F1_CODELETS) $(F1I_CODELETS)

if K7_MODE
ALL_CODELETS = $(K7_CODELETS)
else
ALL_CODELETS = 
endif

EXTRA_DIST = $(K7_CODELETS)
BUILT_SOURCES= $(ALL_CODELETS) $(CODLIST)

libdft_codelets_k7_la_SOURCES = $(BUILT_SOURCES)

SOLVTAB_NAME = X(solvtab_dft_k7)

# special rules for regenerating codelets.
include $(top_srcdir)/support/Makefile.codelets

if MAINTAINER_MODE

GFLAGS = -no-randomized-cse

Kn1k7_%.S:  $(CODELET_DEPS) $(K7GEN_NOTW)
	($(ASMPRELUDE_COMMANDS); $(K7GEN_NOTW) $(GFLAGS) -n $* -name Kn1k7_$*) | $(ADD_DATE)  >$@

Kn1k7i_%.S:  $(CODELET_DEPS) $(K7GEN_NOTW)
	($(ASMPRELUDE_COMMANDS); $(K7GEN_NOTW) $(GFLAGS) -n $* -sign 1 -name Kn1k7i_$*) | $(ADD_DATE)  >$@

Kt1k7_%.S:  $(CODELET_DEPS) $(K7GEN_TWIDDLE)
	($(ASMPRELUDE_COMMANDS); $(K7GEN_TWIDDLE) $(GFLAGS) -n $* -name Kt1k7_$*) | $(ADD_DATE)  >$@

Kt1k7i_%.S:  $(CODELET_DEPS) $(K7GEN_TWIDDLE)
	($(ASMPRELUDE_COMMANDS); $(K7GEN_TWIDDLE) $(GFLAGS) -n $* -sign 1 -name Kt1k7i_$*) | $(ADD_DATE)  >$@

Kf1k7_%.S:  $(CODELET_DEPS) $(K7GEN_TWIDDLE)
	($(ASMPRELUDE_COMMANDS); $(K7GEN_TWIDDLE) $(GFLAGS) -dif -n $* -name Kf1k7_$*) | $(ADD_DATE)  >$@

Kf1k7i_%.S:  $(CODELET_DEPS) $(K7GEN_TWIDDLE)
	($(ASMPRELUDE_COMMANDS); $(K7GEN_TWIDDLE) $(GFLAGS) -dif -n $* -sign 1 -name Kf1k7i_$*) | $(ADD_DATE)  >$@


endif # MAINTAINER_MODE
