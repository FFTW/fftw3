# This Makefile.am specifies a set of codelets, efficient transforms
# of small sizes, that are used as building blocks (kernels) by FFTW
# to build up large transforms, as well as the options for generating
# and compiling them.

# You can customize FFTW for special needs, e.g. to handle certain
# sizes more efficiently, by adding new codelets to the lists of those
# included by default.  If you change the list of codelets, any new
# ones you added will be automatically generated when you run the
# bootstrap script (see "Generating your own code" in the FFTW
# manual).

###########################################################################
AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft	\
-I$(top_srcdir)/dft/codelets
CFLAGS = @CFLAGS@ @CODELET_OPTIM@
noinst_LTLIBRARIES = libdft_inplace.la

###########################################################################
# The F (DIF) codelets are used for a kind of in-place transform algorithm,
# but the planner seems to never (or hardly ever) use them on the machines
# we have access to, preferring the Q codelets and the use of buffers
# for sub-transforms.  So, we comment them out, at least for now.

# f1_<r> is a "twiddle" FFT of size <r>, implementing a radix-r DIF step
F1 = # f1_2.c f1_3.c f1_4.c f1_5.c f1_6.c f1_7.c f1_8.c f1_9.c f1_10.c f1_12.c f1_15.c f1_16.c f1_32.c f1_64.c

# like f1, but partially generates its trig. table on the fly
F2 = # f2_4.c f2_8.c f2_16.c f2_32.c f2_64.c

###########################################################################
# q1_<r> is <r> twiddle FFTs of size <r> (DIF step), where the output is
# transposed.  This is used for in-place transposes in sizes that are
# divisible by <r>^2.  These codelets have size ~ <r>^2, so you should
# probably not use <r> bigger than 8 or so.
Q1 = q1_2.c q1_4.c q1_8.c  q1_3.c q1_5.c q1_6.c

###########################################################################
ALL_CODELETS = $(F1) $(F2) $(Q1)

BUILT_SOURCES=$(ALL_CODELETS) $(CODLIST)

libdft_inplace_la_SOURCES = $(BUILT_SOURCES)

SOLVTAB_NAME = X(solvtab_dft_inplace)

# special rules for regenerating codelets.
include $(top_srcdir)/support/Makefile.codelets

if MAINTAINER_MODE
FLAGS_F1=$(DFT_FLAGS_COMMON)
FLAGS_F2=$(DFT_FLAGS_COMMON) -twiddle-log3
FLAGS_Q1=$(DFT_FLAGS_COMMON) -reload-twiddle
FLAGS_Q2=$(DFT_FLAGS_COMMON) -twiddle-log3

f1_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS_DFT); $(GEN_TWIDDLE) $(FLAGS_F1) -dif -n $* -name f1_$* -include "f.h") | $(ADD_DATE) | $(INDENT) >$@

f2_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS_DFT); $(GEN_TWIDDLE) $(FLAGS_F2) -dif -n $* -name f2_$* -include "f.h") | $(ADD_DATE) | $(INDENT) >$@

q1_%.c:  $(CODELET_DEPS) $(GEN_TWIDSQ)
	($(PRELUDE_COMMANDS_DFT); $(GEN_TWIDSQ) $(FLAGS_Q1) -dif -n $* -name q1_$* -include "q.h") | $(ADD_DATE) | $(INDENT) >$@

q2_%.c:  $(CODELET_DEPS) $(GEN_TWIDSQ)
	($(PRELUDE_COMMANDS_DFT); $(GEN_TWIDSQ) $(FLAGS_Q2) -dif -n $* -name q2_$* -include "q.h") | $(ADD_DATE) | $(INDENT) >$@

endif # MAINTAINER_MODE