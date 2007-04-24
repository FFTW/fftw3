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
-I$(top_srcdir)/rdft/simd -I$(top_srcdir)/simd
AM_CFLAGS = $(SIMD_CFLAGS)
noinst_LTLIBRARIES = librdft_simd_codelets.la

HC2CFDFTV = hc2cfdftv_2.c hc2cfdftv_4.c hc2cfdftv_6.c hc2cfdftv_8.c	\
hc2cfdftv_10.c hc2cfdftv_12.c hc2cfdftv_16.c hc2cfdftv_32.c \
hc2cfdftv_20.c # hc2cfdftv_30.c

HC2CBDFTV = hc2cbdftv_2.c hc2cbdftv_4.c hc2cbdftv_6.c hc2cbdftv_8.c	\
hc2cbdftv_10.c hc2cbdftv_12.c hc2cbdftv_16.c hc2cbdftv_32.c \
hc2cbdftv_20.c # hc2cbdftv_30.c

###########################################################################
SIMD_CODELETS = $(HC2CFDFTV) $(HC2CBDFTV)
if HAVE_SIMD
ALL_CODELETS = $(SIMD_CODELETS)
else
ALL_CODELETS = 
endif

BUILT_SOURCES= $(ALL_CODELETS) $(CODLIST)

librdft_simd_codelets_la_SOURCES = $(BUILT_SOURCES)

SOLVTAB_NAME = X(solvtab_rdft_simd)

# special rules for regenerating codelets.
include $(top_srcdir)/support/Makefile.codelets

if MAINTAINER_MODE
FLAGS_HC2C=-simd $(FLAGS_COMMON) -pipeline-latency 8 -trivial-stores -variables 32 -no-generate-bytw

hc2cfdftv_%.c:  $(CODELET_DEPS) $(GEN_HC2CDFT_C)
	($(PRELUDE_COMMANDS_RDFT); $(TWOVERS) $(GEN_HC2CDFT_C) $(FLAGS_HC2C) -n $* -dit -name hc2cfdftv_$* -include "hc2cfv.h") | $(ADD_DATE) | $(INDENT) >$@

hc2cbdftv_%.c:  $(CODELET_DEPS) $(GEN_HC2CDFT_C)
	($(PRELUDE_COMMANDS_RDFT); $(TWOVERS) $(GEN_HC2CDFT_C) $(FLAGS_HC2C) -n $* -dif -sign 1 -name hc2cbdftv_$* -include "hc2cbv.h") | $(ADD_DATE) | $(INDENT) >$@

endif # MAINTAINER_MODE
