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
noinst_LTLIBRARIES = libdft_standard.la

###########################################################################
# n1_<n> is a hard-coded FFT of size <n> (base cases of FFT recursion)
N1 = n1_2.c n1_3.c n1_4.c n1_5.c n1_6.c n1_7.c n1_8.c n1_9.c n1_10.c	\
n1_11.c n1_12.c n1_13.c n1_14.c n1_15.c n1_16.c

###########################################################################
# m1_<n> is a hard-coded FFT of size <n>, with the loop body in a
# separate function
M1 = m1_16.c m1_32.c m1_64.c

###########################################################################
# t1_<r> is a "twiddle" FFT of size <r>, implementing a radix-r DIT step
T1 = t1_2.c t1_3.c t1_4.c t1_5.c t1_6.c t1_7.c t1_8.c t1_9.c	\
t1_10.c t1_12.c t1_15.c t1_16.c t1_32.c t1_64.c

# t2_<r> is also a twiddle FFT, but instead of using a complete lookup table
# of trig. functions, it partially generates the trig. values on the fly
# (this is faster for large sizes).
T2 = t2_4.c t2_8.c t2_16.c t2_32.c t2_64.c

###########################################################################
# input/output stride-specific versions of the above (see FRANZ notes below).

SN1 = sn1_2_2_2.c sn1_2_4_2.c sn1_2_8_2.c sn1_2_16_2.c sn1_2_32_2.c	\
sn1_2_64_2.c sn1_2_128_2.c sn1_2_256_2.c sn1_2_512_2.c sn1_2_1024_2.c	\
sn1_4_2_2.c sn1_4_4_2.c sn1_4_8_2.c sn1_4_16_2.c sn1_4_32_2.c		\
sn1_4_64_2.c sn1_4_128_2.c sn1_4_256_2.c sn1_4_512_2.c sn1_4_1024_2.c	\
sn1_8_2_2.c sn1_8_4_2.c sn1_8_8_2.c sn1_8_16_2.c sn1_8_32_2.c		\
sn1_8_64_2.c sn1_8_128_2.c sn1_8_256_2.c sn1_8_512_2.c sn1_8_1024_2.c	\
sn1_16_2_2.c sn1_16_4_2.c sn1_16_8_2.c sn1_16_16_2.c sn1_16_32_2.c	\
sn1_16_64_2.c sn1_16_128_2.c sn1_16_256_2.c sn1_16_512_2.c		\
sn1_16_1024_2.c sn1_32_2_2.c sn1_32_4_2.c sn1_32_8_2.c sn1_32_16_2.c	\
sn1_32_32_2.c sn1_32_64_2.c sn1_32_128_2.c sn1_32_256_2.c		\
sn1_32_512_2.c sn1_32_1024_2.c sn1_64_2_2.c sn1_64_4_2.c sn1_64_8_2.c	\
sn1_64_16_2.c sn1_64_32_2.c sn1_64_64_2.c sn1_64_128_2.c		\
sn1_64_256_2.c sn1_64_512_2.c sn1_64_1024_2.c

SN1COD = sn1_16.cod sn1_2.cod sn1_32.cod sn1_4.cod sn1_64.cod	\
sn1_8.cod

ST1 = st1_2_4.c st1_2_8.c st1_2_16.c st1_2_32.c st1_2_64.c		\
st1_2_128.c st1_2_256.c st1_2_512.c st1_2_1024.c st1_4_4.c st1_4_8.c	\
st1_4_16.c st1_4_32.c st1_4_64.c st1_4_128.c st1_4_256.c st1_4_512.c	\
st1_4_1024.c st1_8_4.c st1_8_8.c st1_8_16.c st1_8_32.c st1_8_64.c	\
st1_8_128.c st1_8_256.c st1_8_512.c st1_8_1024.c st1_16_4.c st1_16_8.c	\
st1_16_16.c st1_16_32.c st1_16_64.c st1_16_128.c st1_16_256.c		\
st1_16_512.c st1_16_1024.c st1_32_4.c st1_32_8.c st1_32_16.c		\
st1_32_32.c st1_32_64.c st1_32_128.c st1_32_256.c st1_32_512.c		\
st1_32_1024.c st1_64_4.c st1_64_8.c st1_64_16.c st1_64_32.c		\
st1_64_64.c st1_64_128.c st1_64_256.c st1_64_512.c st1_64_1024.c

ST1COD = st1_16.cod st1_2.cod st1_32.cod st1_4.cod st1_64.cod	\
st1_8.cod

# Uncomment these to include many specialized codelets of fixed
# strides, which can speed things up on Pentia where stride arithmetic
# is expensive, at the expense of increased library size.
FRANZ_CODELETS = # $(SN1) $(ST1)
FRANZ_EXTRA = # $(SN1COD) $(ST1COD)

###########################################################################
ALL_CODELETS = $(N1) $(M1) $(T1) $(T2) $(FRANZ_CODELETS)

BUILT_SOURCES= $(ALL_CODELETS) $(CODLIST) $(FRANZ_EXTRA)

libdft_standard_la_SOURCES = $(BUILT_SOURCES)

SOLVTAB_NAME = X(solvtab_dft_standard)

# special rules for regenerating codelets.
include $(top_srcdir)/support/Makefile.codelets

if MAINTAINER_MODE
FLAGS_N1=$(DFT_FLAGS_COMMON)
FLAGS_T1=$(DFT_FLAGS_COMMON)
FLAGS_T2=$(DFT_FLAGS_COMMON) -twiddle-log3

n1_%.c:  $(CODELET_DEPS) $(GEN_NOTW)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW) $(FLAGS_N1) -n $* -name n1_$* -include "n.h") | $(ADD_DATE) | $(INDENT) >$@

m1_%.c:  $(CODELET_DEPS) $(GEN_NOTW_NOINLINE)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_NOINLINE) $(FLAGS_N1) -n $* -name m1_$* -include "n.h") | $(ADD_DATE) | $(INDENT) >$@


sn1_%.cod:  $(CODELET_DEPS) $(GEN_NOTW)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW) $(FLAGS_N1) -n $* -name "" -with-istride ISTRIDE -with-ostride OSTRIDE -include "n.h") | $(ADD_DATE) | $(INDENT) >$@

sn1_%.c:  $(CODELET_DEPS)
	echo "$*" | tr '_' \\n | (read n; read is; read os;	\
        echo "#define ISTRIDE $$is";				\
        echo "#define OSTRIDE $$os";				\
        echo "#define NAME X(codelet_sn1_$${n}_$${is}_$${os})";\
        echo "#include \"sn1_$${n}.cod\"") >$@

t1_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS_DFT); $(GEN_TWIDDLE) $(FLAGS_T1) -n $* -name t1_$* -include "t.h") | $(ADD_DATE) | $(INDENT) >$@

st1_%.cod:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS_DFT); $(GEN_TWIDDLE) $(FLAGS_T1) -n $* -name "" -with-iostride IOSTRIDE -include "t.h") | $(ADD_DATE) | $(INDENT) >$@

st1_%.c:  $(CODELET_DEPS)
	echo "$*" | tr '_' \\n | (read n; read ios;	\
        echo "#define IOSTRIDE $$ios";				\
        echo "#define NAME X(codelet_st1_$${n}_$${ios})";\
        echo "#include \"st1_$${n}.cod\"") >$@

t2_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS_DFT); $(GEN_TWIDDLE) $(FLAGS_T2) -n $* -name t2_$* -include "t.h") | $(ADD_DATE) | $(INDENT) >$@

endif # MAINTAINER_MODE