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
noinst_LTLIBRARIES = librdft_codelets_hc2r.la

###########################################################################
# hc2r_<n> is a hard-coded complex-to-real FFT of size <n> (base cases
# of real-output FFT recursion)
HC2R = hc2r_3.c hc2r_4.c hc2r_5.c hc2r_6.c hc2r_7.c hc2r_8.c hc2r_9.c	\
hc2r_10.c hc2r_11.c hc2r_12.c hc2r_13.c hc2r_14.c hc2r_15.c hc2r_16.c	\
hc2r_32.c hc2r_64.c hc2r_128.c

# (hc2r_2.c is equivalent to r2hc_2.c)

###########################################################################
# hb_<r> is a "twiddle" FFT of size <r>, implementing a radix-r DIF
# step for a real-output FFT.  Every hb codelet must have a
# corresponding hc2rIII codelet (see below)!
HB = hb_2.c hb_3.c hb_4.c hb_5.c hb_6.c hb_7.c hb_8.c hb_9.c	\
hb_10.c hb_12.c hb_15.c hb_16.c hb_32.c hb_64.c

# like hb, but generates part of its trig table on the fly (good for large n)
HB2 = hb2_4.c hb2_8.c hb2_16.c hb2_32.c hb2_64.c

# an hc2r transform where the output is shifted by half a sample (input
# is multiplied by a phase).  This is needed as part of the DIF recursion;
# every hb_<r> or hb2_<r> codelet should have a corresponding hc2rIII_<r>
HC2RIII = hc2rIII_2.c hc2rIII_3.c hc2rIII_4.c hc2rIII_5.c hc2rIII_6.c	\
hc2rIII_7.c hc2rIII_8.c hc2rIII_9.c hc2rIII_10.c hc2rIII_12.c		\
hc2rIII_15.c hc2rIII_16.c hc2rIII_32.c hc2rIII_64.c

###########################################################################
ALL_CODELETS = $(HC2R) $(HB) $(HC2RIII)

BUILT_SOURCES= $(ALL_CODELETS) $(CODLIST)

librdft_codelets_hc2r_la_SOURCES = $(BUILT_SOURCES)

SOLVTAB_NAME = X(solvtab_rdft_hc2r)

# special rules for regenerating codelets.
include $(top_srcdir)/support/Makefile.codelets

if MAINTAINER_MODE
FLAGS_HC2R=$(RDFT_FLAGS_COMMON) -sign 1
FLAGS_HB=$(RDFT_FLAGS_COMMON) -sign 1
FLAGS_HB2=$(RDFT_FLAGS_COMMON) -sign 1 -twiddle-log3 -precompute-twiddles
FLAGS_HC2RIII=$(RDFT_FLAGS_COMMON) -sign 1

hc2r_%.c:  $(CODELET_DEPS) $(GEN_HC2R)
	($(PRELUDE_COMMANDS_RDFT); $(TWOVERS) $(GEN_HC2R) $(FLAGS_HC2R) -n $* -name hc2r_$* -include "hc2r.h") | $(ADD_DATE) | $(INDENT) >$@

hb_%.c:  $(CODELET_DEPS) $(GEN_HC2HC)
	($(PRELUDE_COMMANDS_RDFT); $(TWOVERS) $(GEN_HC2HC) $(FLAGS_HB) -n $* -dif -name hb_$* -include "hb.h") | $(ADD_DATE) | $(INDENT) >$@

hb2_%.c:  $(CODELET_DEPS) $(GEN_HC2HC)
	($(PRELUDE_COMMANDS_RDFT); $(TWOVERS) $(GEN_HC2HC) $(FLAGS_HB2) -n $* -dif -name hb2_$* -include "hb.h") | $(ADD_DATE) | $(INDENT) >$@

hc2rIII_%.c:  $(CODELET_DEPS) $(GEN_HC2R)
	($(PRELUDE_COMMANDS_RDFT); $(TWOVERS) $(GEN_HC2R) $(FLAGS_HC2R) -n $* -name hc2rIII_$* -dft-III -include "hc2rIII.h") | $(ADD_DATE) | $(INDENT) >$@

endif # MAINTAINER_MODE