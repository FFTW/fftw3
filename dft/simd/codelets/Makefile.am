AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/dft	\
-I$(top_srcdir)/dft/simd -I$(top_srcdir)/simd
CFLAGS = @CFLAGS@ @CODELET_OPTIM@ $(SIMD_CFLAGS)
noinst_LTLIBRARIES = libdft_codelets_simd.la

N1F = n1fv_2.c n1fv_3.c n1fv_4.c n1fv_5.c n1fv_6.c n1fv_7.c n1fv_8.c	\
n1fv_9.c n1fv_10.c n1fv_11.c n1fv_12.c n1fv_13.c n1fv_14.c n1fv_15.c	\
n1fv_16.c n1fv_32.c n1fv_64.c

N1B = n1bv_2.c n1bv_3.c n1bv_4.c n1bv_5.c n1bv_6.c n1bv_7.c n1bv_8.c	\
n1bv_9.c n1bv_10.c n1bv_11.c n1bv_12.c n1bv_13.c n1bv_14.c n1bv_15.c	\
n1bv_16.c n1bv_32.c n1bv_64.c

T1F = t1fv_2.c t1fv_3.c t1fv_4.c t1fv_5.c t1fv_6.c t1fv_7.c t1fv_8.c	\
t1fv_9.c t1fv_10.c t1fv_12.c t1fv_15.c t1fv_16.c t1fv_32.c t1fv_64.c

T1B = t1bv_2.c t1bv_3.c t1bv_4.c t1bv_5.c t1bv_6.c t1bv_7.c t1bv_8.c	\
t1bv_9.c t1bv_10.c t1bv_12.c t1bv_15.c t1bv_16.c t1bv_32.c t1bv_64.c

Q1F = q1fv_2.c q1fv_4.c q1fv_8.c

Q1B = q1bv_2.c q1bv_4.c q1bv_8.c

SN1F = sn1fv_2_4_2.c sn1fv_2_8_2.c sn1fv_2_16_2.c sn1fv_2_32_2.c	\
sn1fv_2_64_2.c sn1fv_2_128_2.c sn1fv_2_256_2.c sn1fv_2_512_2.c		\
sn1fv_2_1024_2.c sn1fv_4_4_2.c sn1fv_4_8_2.c sn1fv_4_16_2.c		\
sn1fv_4_32_2.c sn1fv_4_64_2.c sn1fv_4_128_2.c sn1fv_4_256_2.c		\
sn1fv_4_512_2.c sn1fv_4_1024_2.c sn1fv_8_4_2.c sn1fv_8_8_2.c		\
sn1fv_8_16_2.c sn1fv_8_32_2.c sn1fv_8_64_2.c sn1fv_8_128_2.c		\
sn1fv_8_256_2.c sn1fv_8_512_2.c sn1fv_8_1024_2.c sn1fv_16_4_2.c		\
sn1fv_16_8_2.c sn1fv_16_16_2.c sn1fv_16_32_2.c sn1fv_16_64_2.c		\
sn1fv_16_128_2.c sn1fv_16_256_2.c sn1fv_16_512_2.c sn1fv_16_1024_2.c	\
sn1fv_32_4_2.c sn1fv_32_8_2.c sn1fv_32_16_2.c sn1fv_32_32_2.c		\
sn1fv_32_64_2.c sn1fv_32_128_2.c sn1fv_32_256_2.c sn1fv_32_512_2.c	\
sn1fv_32_1024_2.c sn1fv_64_4_2.c sn1fv_64_8_2.c sn1fv_64_16_2.c		\
sn1fv_64_32_2.c sn1fv_64_64_2.c sn1fv_64_128_2.c sn1fv_64_256_2.c	\
sn1fv_64_512_2.c sn1fv_64_1024_2.c

SN1FCOD = sn1fv_16.cod sn1fv_2.cod sn1fv_32.cod sn1fv_4.cod	\
sn1fv_64.cod sn1fv_8.cod

ST1F = st1fv_2_4.c st1fv_2_8.c st1fv_2_16.c st1fv_2_32.c st1fv_2_64.c	\
st1fv_2_128.c st1fv_2_256.c st1fv_2_512.c st1fv_2_1024.c st1fv_4_4.c	\
st1fv_4_8.c st1fv_4_16.c st1fv_4_32.c st1fv_4_64.c st1fv_4_128.c	\
st1fv_4_256.c st1fv_4_512.c st1fv_4_1024.c st1fv_8_4.c st1fv_8_8.c	\
st1fv_8_16.c st1fv_8_32.c st1fv_8_64.c st1fv_8_128.c st1fv_8_256.c	\
st1fv_8_512.c st1fv_8_1024.c st1fv_16_4.c st1fv_16_8.c st1fv_16_16.c	\
st1fv_16_32.c st1fv_16_64.c st1fv_16_128.c st1fv_16_256.c		\
st1fv_16_512.c st1fv_16_1024.c st1fv_32_4.c st1fv_32_8.c st1fv_32_16.c	\
st1fv_32_32.c st1fv_32_64.c st1fv_32_128.c st1fv_32_256.c		\
st1fv_32_512.c st1fv_32_1024.c st1fv_64_4.c st1fv_64_8.c st1fv_64_16.c	\
st1fv_64_32.c st1fv_64_64.c st1fv_64_128.c st1fv_64_256.c		\
st1fv_64_512.c st1fv_64_1024.c

ST1FCOD = st1fv_16.cod st1fv_2.cod st1fv_32.cod st1fv_4.cod	\
st1fv_64.cod st1fv_8.cod

# Uncomment these to include specialized codelets of many fixed strides,
# which can speed things up on Pentia where stride arithmetic is expensive,
# at the expense of increased library size.
FRANZ_CODELETS = # $(SN1F) $(ST1F)
FRANZ_EXTRA = # $(SN1FCOD) $(ST1FCOD)

SIMD_CODELETS = $(N1F) $(N1B) $(T1F) $(T1B) $(Q1F) $(Q1B) $(FRANZ_CODELETS)

if HAVE_SIMD
ALL_CODELETS = $(SIMD_CODELETS)
else
ALL_CODELETS = 
endif

EXTRA_DIST = $(SIMD_CODELETS) $(FRANZ_EXTRA)
BUILT_SOURCES= $(ALL_CODELETS) $(CODLIST) $(FRANZ_EXTRA)

libdft_codelets_simd_la_SOURCES = $(BUILT_SOURCES)

SOLVTAB_NAME = X(solvtab_dft_simd)

# special rules for regenerating codelets.
include $(top_srcdir)/support/Makefile.codelets

if MAINTAINER_MODE
GFLAGS = -simd -trivial-stores $(FLAGS_COMMON)

n1fv_%.c:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_C) $(GFLAGS) -n $* -name n1fv_$* -include "n1f.h") | $(ADD_DATE) | $(INDENT) >$@

sn1fv_%.cod:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_C) $(GFLAGS) -n $* -name "" -with-istride ISTRIDE -with-ostride OSTRIDE -include "n1f.h") | $(ADD_DATE) | $(INDENT) >$@

sn1fv_%.c:  $(CODELET_DEPS)
	echo "$*" | tr '_' \\n | (read n; read is; read os;	\
        echo "#define ISTRIDE $$is";				\
        echo "#define OSTRIDE $$os";				\
        echo "#define NAME X(codelet_sn1fv_$${n}_$${is}_$${os})";\
        echo "#include \"sn1fv_$${n}.cod\"") >$@

n1bv_%.c:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_C) $(GFLAGS) -sign 1 -n $* -name n1bv_$* -include "n1b.h") | $(ADD_DATE) | $(INDENT) >$@

t1fv_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_TWIDDLE_C) $(GFLAGS) -n $* -name t1fv_$* -include "t1f.h") | $(ADD_DATE) | $(INDENT) >$@

st1fv_%.cod:  $(CODELET_DEPS) $(GEN_TWIDDLE_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_TWIDDLE_C) $(GFLAGS) -n $* -name "" -with-iostride IOSTRIDE -include "t1f.h") | $(ADD_DATE) | $(INDENT) >$@

st1fv_%.c:  $(CODELET_DEPS)
	echo "$*" | tr '_' \\n | (read n; read ios;	\
        echo "#define IOSTRIDE $$ios";				\
        echo "#define NAME X(codelet_st1fv_$${n}_$${ios})";\
        echo "#include \"st1fv_$${n}.cod\"") >$@


t1bv_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_TWIDDLE_C) $(GFLAGS) -n $* -name t1bv_$* -include "t1b.h" -sign 1) | $(ADD_DATE) | $(INDENT) >$@

q1fv_%.c:  $(CODELET_DEPS) $(GEN_TWIDSQ_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_TWIDSQ_C) $(GFLAGS) -n $* -dif -name q1fv_$* -include "q1f.h") | $(ADD_DATE) | $(INDENT) >$@

q1bv_%.c:  $(CODELET_DEPS) $(GEN_TWIDSQ_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_TWIDSQ_C) $(GFLAGS) -n $* -dif -name q1bv_$* -include "q1b.h" -sign 1) | $(ADD_DATE) | $(INDENT) >$@


endif # MAINTAINER_MODE
