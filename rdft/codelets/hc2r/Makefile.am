AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/rdft	\
-I$(top_srcdir)/rdft/codelets
CFLAGS = @CFLAGS@ @CODELET_OPTIM@
noinst_LTLIBRARIES = librdft_codelets_hc2r.la

HC2R = hc2r_2.c hc2r_3.c hc2r_4.c hc2r_5.c hc2r_6.c hc2r_7.c hc2r_8.c	\
hc2r_9.c hc2r_10.c hc2r_11.c hc2r_12.c hc2r_13.c hc2r_14.c hc2r_15.c	\
hc2r_16.c hc2r_32.c hc2r_64.c

HB = hb_2.c hb_3.c hb_4.c hb_5.c hb_6.c hb_7.c hb_8.c hb_9.c	\
hb_10.c hb_16.c hb_32.c hb_64.c

HC2RIII = hc2rIII_2.c hc2rIII_3.c hc2rIII_4.c hc2rIII_5.c hc2rIII_6.c	\
hc2rIII_7.c hc2rIII_8.c hc2rIII_9.c hc2rIII_10.c hc2rIII_16.c		\
hc2rIII_32.c hc2rIII_64.c

ALL_CODELETS = $(HC2R) $(HB) $(HC2RIII)

BUILT_SOURCES= $(ALL_CODELETS) $(CODLIST)

librdft_codelets_hc2r_la_SOURCES = $(BUILT_SOURCES)

SOLVTAB_NAME = X(solvtab_rdft_hc2r)

# special rules for regenerating codelets.
include $(top_srcdir)/support/Makefile.codelets

if MAINTAINER_MODE
FLAGS_HC2R=$(RDFT_FLAGS_COMMON) -sign 1
FLAGS_HB=$(RDFT_FLAGS_COMMON) -sign 1
FLAGS_HC2RIII=$(RDFT_FLAGS_COMMON) -sign 1

hc2r_%.c:  $(CODELET_DEPS) $(GEN_HC2R)
	($(PRELUDE_COMMANDS); $(GEN_HC2R) $(FLAGS_HC2R) -n $* -name hc2r_$* -include "hc2r.h") | $(ADD_DATE) | $(INDENT) >$@

hb_%.c:  $(CODELET_DEPS) $(GEN_HC2HC)
	($(PRELUDE_COMMANDS); $(GEN_HC2HC) $(FLAGS_HB) -n $* -dif -name hb_$* -include "hb.h") | $(ADD_DATE) | $(INDENT) >$@

hc2rIII_%.c:  $(CODELET_DEPS) $(GEN_HC2R)
	($(PRELUDE_COMMANDS); $(GEN_HC2R) $(FLAGS_HC2R) -n $* -name hc2rIII_$* -dft-III -include "hc2rIII.h") | $(ADD_DATE) | $(INDENT) >$@

endif # MAINTAINER_MODE