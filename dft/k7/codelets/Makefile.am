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
AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft
noinst_LTLIBRARIES = libdft_codelets_k7.la

# handle automake-1.5
AM_ASFLAGS = $(DEFAULT_INCLUDES)
# handle automake-1.6
AM_CCASFLAGS = $(DEFAULT_INCLUDES)

###########################################################################
# n1k7_<n> is a hard-coded FFTW_FORWARD FFT of size <n>, using K7 assembly
N1_CODELETS = n1k7_2.S n1k7_3.S n1k7_4.S n1k7_5.S n1k7_6.S n1k7_7.S	\
n1k7_8.S n1k7_9.S n1k7_10.S n1k7_11.S n1k7_12.S n1k7_13.S n1k7_14.S	\
n1k7_15.S n1k7_16.S n1k7_32.S n1k7_64.S n1k7_128.S

# as above, but FFTW_BACKWARD
N1I_CODELETS = n1k7i_2.S n1k7i_3.S n1k7i_4.S n1k7i_5.S n1k7i_6.S	\
n1k7i_7.S n1k7i_8.S n1k7i_9.S n1k7i_10.S n1k7i_11.S n1k7i_12.S		\
n1k7i_13.S n1k7i_14.S n1k7i_15.S n1k7i_16.S n1k7i_32.S n1k7i_64.S	\
n1k7i_128.S

###########################################################################
# t1k7_<r> is a "twiddle" FFT of size <r>, implementing a radix-r DIT step
# for an FFTW_FORWARD transform, using K7 assembly
T1_CODELETS = t1k7_2.S t1k7_3.S t1k7_4.S t1k7_5.S t1k7_6.S t1k7_7.S	\
t1k7_8.S t1k7_9.S t1k7_10.S t1k7_12.S t1k7_15.S t1k7_16.S t1k7_32.S	\
t1k7_64.S

# as above, but FFTW_BACKWARD
T1I_CODELETS = t1k7i_2.S t1k7i_3.S t1k7i_4.S t1k7i_5.S t1k7i_6.S	\
t1k7i_7.S t1k7i_8.S t1k7i_9.S t1k7i_10.S t1k7i_12.S t1k7i_15.S		\
t1k7i_16.S t1k7i_32.S t1k7i_64.S

###########################################################################
# f1k7_<r> is a "twiddle" FFT of size <r>, implementing a radix-r DIF step
# for an FFTW_FORWARD transform, using K7 assembly
F1_CODELETS = f1k7_2.S f1k7_4.S f1k7_8.S f1k7_16.S f1k7_32.S f1k7_64.S

# as above, but FFTW_BACKWARD
F1I_CODELETS = f1k7i_2.S f1k7i_4.S f1k7i_8.S f1k7i_16.S f1k7i_32.S	\
f1k7i_64.S

###########################################################################
K7_CODELETS = $(N1_CODELETS) $(N1I_CODELETS) $(T1_CODELETS) $(T1I_CODELETS) $(F1_CODELETS) $(F1I_CODELETS)

if HAVE_K7
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

n1k7_%.S:  $(CODELET_DEPS) $(K7GEN_NOTW)
	($(ASMPRELUDE_COMMANDS); $(K7GEN_NOTW) $(GFLAGS) -n $* -name n1k7_$*) | $(ADD_DATE)  >$@

n1k7i_%.S:  $(CODELET_DEPS) $(K7GEN_NOTW)
	($(ASMPRELUDE_COMMANDS); $(K7GEN_NOTW) $(GFLAGS) -n $* -sign 1 -name n1k7i_$*) | $(ADD_DATE)  >$@

t1k7_%.S:  $(CODELET_DEPS) $(K7GEN_TWIDDLE)
	($(ASMPRELUDE_COMMANDS); $(K7GEN_TWIDDLE) $(GFLAGS) -n $* -name t1k7_$*) | $(ADD_DATE)  >$@

t1k7i_%.S:  $(CODELET_DEPS) $(K7GEN_TWIDDLE)
	($(ASMPRELUDE_COMMANDS); $(K7GEN_TWIDDLE) $(GFLAGS) -n $* -sign 1 -name t1k7i_$*) | $(ADD_DATE)  >$@

f1k7_%.S:  $(CODELET_DEPS) $(K7GEN_TWIDDLE)
	($(ASMPRELUDE_COMMANDS); $(K7GEN_TWIDDLE) $(GFLAGS) -dif -n $* -name f1k7_$*) | $(ADD_DATE)  >$@

f1k7i_%.S:  $(CODELET_DEPS) $(K7GEN_TWIDDLE)
	($(ASMPRELUDE_COMMANDS); $(K7GEN_TWIDDLE) $(GFLAGS) -dif -n $* -sign 1 -name f1k7i_$*) | $(ADD_DATE)  >$@


endif # MAINTAINER_MODE
