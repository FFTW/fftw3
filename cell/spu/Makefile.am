noinst_PROGRAMS=spufftw planner

# FIXME: autodetect SPUCC
SPUCC=spu-gcc
SPUCFLAGS= -O3 -W -Wall -Winline -Wno-main -I. -I$(srcdir)	\
-I$(top_builddir) -I${top_srcdir}/kernel

# use the silly suffix .spuc so that automake does not get 
# confused
.spuc.o:
	$(SPUCC) -xc $(SPUCFLAGS) -c -o $@ $<

DOUBLE_CODELETS=spu_n1fv_3.spuc spu_n1fv_5.spuc spu_n1fv_7.spuc		\
spu_n1fv_9.spuc spu_n1fv_11.spuc spu_n1fv_13.spuc spu_n1fv_15.spuc

SINGLE_CODELETS=spu_n2fv_2.spuc spu_n2fv_4.spuc spu_n2fv_6.spuc		\
spu_n2fv_8.spuc spu_n2fv_10.spuc spu_n2fv_12.spuc spu_n2fv_14.spuc	\
spu_n2fv_16.spuc spu_n2fv_32.spuc spu_t1fv_2.spuc spu_t1fv_3.spuc	\
spu_t1fv_4.spuc spu_t1fv_5.spuc spu_t1fv_6.spuc spu_t1fv_7.spuc		\
spu_t1fv_8.spuc spu_t1fv_9.spuc spu_t1fv_10.spuc spu_t1fv_12.spuc	\
spu_t1fv_15.spuc spu_t1fv_16.spuc spu_t1fv_32.spuc

if SINGLE
ALL_CODELETS=$(SINGLE_CODELETS)
else
ALL_CODELETS=$(SINGLE_CODELETS) $(DOUBLE_CODELETS)
endif

BUILT_SOURCES= $(SINGLE_CODELETS) $(DOUBLE_CODELETS)
HFILES=fftw-spu.h spu-double.h spu-single.h
OTHERDEPS=${top_srcdir}/kernel/ifftw.h ${top_srcdir}/cell/fftw-cell.h	\
${top_builddir}/config.h

# override the value set by configure
LIBS=-lm

spufftw_SOURCES=main.spuc alloc.spuc copy.spuc dma.spuc execute.spuc	\
transpose.spuc dft.spuc $(ALL_CODELETS) $(HFILES)
spufftw_LINK=$(SPUCC) -o $@
$(spufftw_OBJECTS): $(HFILES) $(OTHERDEPS)

planner_SOURCES=planner.spuc execute.spuc $(ALL_CODELETS) $(HFILES)
planner_LINK=$(SPUCC) -o $@

# special rules for regenerating codelets.
include $(top_srcdir)/support/Makefile.codelets

if MAINTAINER_MODE
FLAGS_N=-standalone -fma -reorder-insns -simd -compact -variables 100000 -with-ostride 2 -include fftw-spu.h
FLAGS_T=-standalone -fma -reorder-insns -simd -compact -variables 100000 -include fftw-spu.h -trivial-stores

PRELUDE_COMMANDS=cat $(COPYRIGHT)

spu_n1fv_%.spuc:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS); $(GEN_NOTW_C) $(FLAGS_N) -n $* -name "X(spu_n2fv_$*)") | $(ADD_DATE) | $(INDENT) >$@

spu_n2fv_%.spuc:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS); $(GEN_NOTW_C) $(FLAGS_N) -store-multiple 2 -n $* -name "X(spu_n2fv_$*)") | $(ADD_DATE) | $(INDENT) >$@

spu_t1fv_%.spuc:  $(CODELET_DEPS) $(GEN_TWIDDLE_C)
	($(PRELUDE_COMMANDS); $(GEN_TWIDDLE_C) $(FLAGS_T) -n $* -name "X(spu_t1fv_$*)") | $(ADD_DATE) | $(INDENT) >$@

endif # MAINTAINER_MODE
