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
AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/rdft	\
-I$(top_srcdir)/rdft/codelets
noinst_LTLIBRARIES = librdft_codelets_r2hc.la

###########################################################################
# r2hc_<n> is a hard-coded real-to-complex FFT of size <n> (base cases
# of real-input FFT recursion)
R2HC = r2hc_2.c r2hc_3.c r2hc_4.c r2hc_5.c r2hc_6.c r2hc_7.c r2hc_8.c	\
r2hc_9.c r2hc_10.c r2hc_11.c r2hc_12.c r2hc_13.c r2hc_14.c r2hc_15.c	\
r2hc_16.c r2hc_32.c r2hc_64.c r2hc_128.c 

###########################################################################
# hf_<r> is a "twiddle" FFT of size <r>, implementing a radix-r DIT
# step for a real-input FFT.  Every hf codelet must have a
# corresponding r2hcII codelet (see below)!
HF = hf_2.c hf_3.c hf_4.c hf_5.c hf_6.c hf_7.c hf_8.c hf_9.c	\
hf_10.c hf_12.c hf_15.c hf_16.c hf_32.c hf_64.c

# like hf, but generates part of its trig table on the fly (good for large n)
HF2 = hf2_4.c hf2_8.c hf2_16.c hf2_32.c hf2_64.c

# an r2hc transform where the input is shifted by half a sample (output
# is multiplied by a phase).  This is needed as part of the DIT recursion;
# every hf_<r> or hf2_<r> codelet should have a corresponding r2hcII_<r>
R2HCII = r2hcII_2.c r2hcII_3.c r2hcII_4.c r2hcII_5.c r2hcII_6.c		\
r2hcII_7.c r2hcII_8.c r2hcII_9.c r2hcII_10.c r2hcII_12.c r2hcII_15.c	\
r2hcII_16.c r2hcII_32.c r2hcII_64.c

###########################################################################
ALL_CODELETS = $(R2HC) $(HF) $(HF2) $(R2HCII)

BUILT_SOURCES= $(ALL_CODELETS) $(CODLIST)

librdft_codelets_r2hc_la_SOURCES = $(BUILT_SOURCES)

SOLVTAB_NAME = X(solvtab_rdft_r2hc)

# special rules for regenerating codelets.
include $(top_srcdir)/support/Makefile.codelets

if MAINTAINER_MODE
FLAGS_R2HC=$(RDFT_FLAGS_COMMON)
FLAGS_HF=$(RDFT_FLAGS_COMMON)
FLAGS_HF2=$(RDFT_FLAGS_COMMON) -twiddle-log3 -precompute-twiddles
FLAGS_R2HCII=$(RDFT_FLAGS_COMMON)

r2hc_%.c:  $(CODELET_DEPS) $(GEN_R2HC)
	($(PRELUDE_COMMANDS_RDFT); $(TWOVERS) $(GEN_R2HC) $(FLAGS_R2HC) -n $* -name r2hc_$* -include "r2hc.h") | $(ADD_DATE) | $(INDENT) >$@

hf_%.c:  $(CODELET_DEPS) $(GEN_HC2HC)
	($(PRELUDE_COMMANDS_RDFT); $(TWOVERS) $(GEN_HC2HC) $(FLAGS_HF) -n $* -dit -name hf_$* -include "hf.h") | $(ADD_DATE) | $(INDENT) >$@

hf2_%.c:  $(CODELET_DEPS) $(GEN_HC2HC)
	($(PRELUDE_COMMANDS_RDFT); $(TWOVERS) $(GEN_HC2HC) $(FLAGS_HF2) -n $* -dit -name hf2_$* -include "hf.h") | $(ADD_DATE) | $(INDENT) >$@

r2hcII_%.c:  $(CODELET_DEPS) $(GEN_R2HC)
	($(PRELUDE_COMMANDS_RDFT); $(TWOVERS) $(GEN_R2HC) $(FLAGS_R2HC) -n $* -name r2hcII_$* -dft-II -include "r2hcII.h") | $(ADD_DATE) | $(INDENT) >$@

endif # MAINTAINER_MODE