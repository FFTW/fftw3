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
-I$(top_srcdir)/dft/simd -I$(top_srcdir)/simd
CFLAGS = @CFLAGS@ @CODELET_OPTIM@ $(SIMD_CFLAGS)
noinst_LTLIBRARIES = libdft_codelets_simd.la

###########################################################################
# n1fv_<n> is a hard-coded FFTW_FORWARD FFT of size <n>, using SIMD
N1F = n1fv_2.c n1fv_3.c n1fv_4.c n1fv_5.c n1fv_6.c n1fv_7.c n1fv_8.c	\
n1fv_9.c n1fv_10.c n1fv_11.c n1fv_12.c n1fv_13.c n1fv_14.c n1fv_15.c	\
n1fv_16.c n1fv_32.c n1fv_64.c

# as above, but FFTW_BACKWARD
N1B = n1bv_2.c n1bv_3.c n1bv_4.c n1bv_5.c n1bv_6.c n1bv_7.c n1bv_8.c	\
n1bv_9.c n1bv_10.c n1bv_11.c n1bv_12.c n1bv_13.c n1bv_14.c n1bv_15.c	\
n1bv_16.c n1bv_32.c n1bv_64.c

# as above, with arbitrary input vector stride
N2F = n2fv_2.c n2fv_3.c n2fv_4.c n2fv_5.c n2fv_6.c n2fv_7.c n2fv_8.c	\
n2fv_9.c n2fv_10.c n2fv_11.c n2fv_12.c n2fv_13.c n2fv_14.c n2fv_15.c	\
n2fv_16.c n2fv_32.c n2fv_64.c

N2B = n2bv_2.c n2bv_3.c n2bv_4.c n2bv_5.c n2bv_6.c n2bv_7.c n2bv_8.c	\
n2bv_9.c n2bv_10.c n2bv_11.c n2bv_12.c n2bv_13.c n2bv_14.c n2bv_15.c	\
n2bv_16.c n2bv_32.c n2bv_64.c

# common code to both n1 and n2
NFCOD = nfv_2.cod nfv_3.cod nfv_4.cod nfv_5.cod nfv_6.cod nfv_7.cod	\
nfv_8.cod nfv_9.cod nfv_10.cod nfv_11.cod nfv_12.cod nfv_13.cod		\
nfv_14.cod nfv_15.cod nfv_16.cod nfv_32.cod nfv_64.cod

NBCOD = nbv_2.cod nbv_3.cod nbv_4.cod nbv_5.cod nbv_6.cod nbv_7.cod	\
nbv_8.cod nbv_9.cod nbv_10.cod nbv_11.cod nbv_12.cod nbv_13.cod		\
nbv_14.cod nbv_15.cod nbv_16.cod nbv_32.cod nbv_64.cod

###########################################################################
# m* codelets are like n* codelets, but with the loop body in
# a separate function.  This nonsense is necessary, compilers
# being what they are.
M1F = m1fv_2.c m1fv_3.c m1fv_4.c m1fv_5.c m1fv_6.c m1fv_7.c m1fv_8.c	\
m1fv_9.c m1fv_10.c m1fv_11.c m1fv_12.c m1fv_13.c m1fv_14.c m1fv_15.c	\
m1fv_16.c m1fv_32.c m1fv_64.c

# as above, but FFTW_BACKWARD
M1B = m1bv_2.c m1bv_3.c m1bv_4.c m1bv_5.c m1bv_6.c m1bv_7.c m1bv_8.c	\
m1bv_9.c m1bv_10.c m1bv_11.c m1bv_12.c m1bv_13.c m1bv_14.c m1bv_15.c	\
m1bv_16.c m1bv_32.c m1bv_64.c

# as above, with arbitrary imput vector stride
M2F = m2fv_2.c m2fv_3.c m2fv_4.c m2fv_5.c m2fv_6.c m2fv_7.c m2fv_8.c	\
m2fv_9.c m2fv_10.c m2fv_11.c m2fv_12.c m2fv_13.c m2fv_14.c m2fv_15.c	\
m2fv_16.c m2fv_32.c m2fv_64.c

M2B = m2bv_2.c m2bv_3.c m2bv_4.c m2bv_5.c m2bv_6.c m2bv_7.c m2bv_8.c	\
m2bv_9.c m2bv_10.c m2bv_11.c m2bv_12.c m2bv_13.c m2bv_14.c m2bv_15.c	\
m2bv_16.c m2bv_32.c m2bv_64.c

# commom code to both m1 amd m2
MFCOD = mfv_2.cod mfv_3.cod mfv_4.cod mfv_5.cod mfv_6.cod mfv_7.cod	\
mfv_8.cod mfv_9.cod mfv_10.cod mfv_11.cod mfv_12.cod mfv_13.cod		\
mfv_14.cod mfv_15.cod mfv_16.cod mfv_32.cod mfv_64.cod

MBCOD = mbv_2.cod mbv_3.cod mbv_4.cod mbv_5.cod mbv_6.cod mbv_7.cod	\
mbv_8.cod mbv_9.cod mbv_10.cod mbv_11.cod mbv_12.cod mbv_13.cod		\
mbv_14.cod mbv_15.cod mbv_16.cod mbv_32.cod mbv_64.cod

###########################################################################
# t1fv_<r> is a "twiddle" FFT of size <r>, implementing a radix-r DIT step
# for an FFTW_FORWARD transform, using SIMD
T1F = t1fv_2.c t1fv_3.c t1fv_4.c t1fv_5.c t1fv_6.c t1fv_7.c t1fv_8.c	\
t1fv_9.c t1fv_10.c t1fv_12.c t1fv_15.c t1fv_16.c t1fv_32.c t1fv_64.c

# as above, but FFTW_BACKWARD
T1B = t1bv_2.c t1bv_3.c t1bv_4.c t1bv_5.c t1bv_6.c t1bv_7.c t1bv_8.c	\
t1bv_9.c t1bv_10.c t1bv_12.c t1bv_15.c t1bv_16.c t1bv_32.c t1bv_64.c

###########################################################################
# q1fv_<r> is <r> twiddle FFTW_FORWARD FFTs of size <r> (DIF step),
# where the output is transposed, using SIMD.  This is used for
# in-place transposes in sizes that are divisible by <r>^2.  These
# codelets have size ~ <r>^2, so you should probably not use <r>
# bigger than 8 or so.
Q1F = q1fv_2.c q1fv_4.c q1fv_8.c

# as above, but FFTW_BACKWARD
Q1B = q1bv_2.c q1bv_4.c q1bv_8.c

###########################################################################
# input/output stride-specific versions of the above (see FRANZ notes below).

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

# Uncomment these to include many specialized codelets of fixed
# strides, which can speed things up on Pentia where stride arithmetic
# is expensive, as well as improving 4-way SIMD efficiency for the
# unit-stride cases, at the expense of increased library size.
FRANZ_CODELETS = # $(SN1F) $(ST1F)
FRANZ_EXTRA = # $(SN1FCOD) $(ST1FCOD)

###########################################################################
SIMD_EXTRA = $(NFCOD) $(NBCOD) $(MFCOD) $(MBCOD)
SIMD_CODELETS = $(N1F) $(N1B) $(M1F) $(M1B) $(T1F) $(T1B) $(Q1F)	\
$(Q1B) $(FRANZ_CODELETS)
SIMD4_CODELETS = $(N2F) $(N2B) $(M2F) $(M2B)

if HAVE_SIMD4
ALL4_CODELETS = $(SIMD4_CODELETS)
else
ALL4_CODELETS =
endif

if HAVE_SIMD
ALL_CODELETS = $(SIMD_CODELETS) $(ALL4_CODELETS)
else
ALL_CODELETS = 
endif

EXTRA_DIST = $(SIMD_CODELETS) $(SIMD4_CODELETS) $(SIMD_EXTRA) $(FRANZ_EXTRA)
BUILT_SOURCES= $(ALL_CODELETS) $(CODLIST) $(SIMD_EXTRA) $(FRANZ_EXTRA)

libdft_codelets_simd_la_SOURCES = $(BUILT_SOURCES)

SOLVTAB_NAME = X(solvtab_dft_simd)

# special rules for regenerating codelets.
include $(top_srcdir)/support/Makefile.codelets

if MAINTAINER_MODE
GFLAGS = -simd -trivial-stores $(FLAGS_COMMON)

nfv_%.cod:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_C) $(GFLAGS) -n $* -name "") | $(ADD_DATE) | $(INDENT) >$@

nbv_%.cod:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_C) $(GFLAGS) -sign 1 -n $* -name "") | $(ADD_DATE) | $(INDENT) >$@

n1fv_%.c:  $(CODELET_DEPS)						
	(echo "#include \"n1f.h\"";			\
	n=$*; echo "#define NAME X(codelet_n1fv_$$n)";	\
        echo "#include \"nfv_$${n}.cod\"") >$@

n2fv_%.c:  $(CODELET_DEPS)						
	(echo "#include \"n2f.h\"";			\
	n=$*; echo "#define NAME X(codelet_n2fv_$$n)";	\
        echo "#include \"nfv_$${n}.cod\"") >$@

n1bv_%.c:  $(CODELET_DEPS)						
	(echo "#include \"n1b.h\"";			\
	n=$*; echo "#define NAME X(codelet_n1bv_$$n)";	\
        echo "#include \"nbv_$${n}.cod\"") >$@

n2bv_%.c:  $(CODELET_DEPS)						
	(echo "#include \"n2b.h\"";			\
	n=$*; echo "#define NAME X(codelet_n2bv_$$n)";	\
        echo "#include \"nbv_$${n}.cod\"") >$@

nfv_%.cod:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_C) $(GFLAGS) -n $* -name "") | $(ADD_DATE) | $(INDENT) >$@

nbv_%.cod:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_C) $(GFLAGS) -sign 1 -n $* -name "") | $(ADD_DATE) | $(INDENT) >$@

m1fv_%.c:  $(CODELET_DEPS)						
	(echo "#include \"n1f.h\"";			\
	n=$*; echo "#define NAME X(codelet_m1fv_$$n)";	\
        echo "#include \"mfv_$${n}.cod\"") >$@

m2fv_%.c:  $(CODELET_DEPS)						
	(echo "#include \"n2f.h\"";			\
	n=$*; echo "#define NAME X(codelet_m2fv_$$n)";	\
        echo "#include \"mfv_$${n}.cod\"") >$@

m1bv_%.c:  $(CODELET_DEPS)						
	(echo "#include \"n1b.h\"";			\
	n=$*; echo "#define NAME X(codelet_m1bv_$$n)";	\
        echo "#include \"mbv_$${n}.cod\"") >$@

m2bv_%.c:  $(CODELET_DEPS)						
	(echo "#include \"n2b.h\"";			\
	n=$*; echo "#define NAME X(codelet_m2bv_$$n)";	\
        echo "#include \"mbv_$${n}.cod\"") >$@

mfv_%.cod:  $(CODELET_DEPS) $(GEN_NOTW_NOINLINE_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_NOINLINE_C) $(GFLAGS) -n $* -name "") | $(ADD_DATE) | $(INDENT) >$@

mbv_%.cod:  $(CODELET_DEPS) $(GEN_NOTW_NOINLINE_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_NOINLINE_C) $(GFLAGS) -sign 1 -n $* -name "") | $(ADD_DATE) | $(INDENT) >$@


sn1fv_%.cod:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_C) $(GFLAGS) -n $* -name "" -with-istride ISTRIDE -with-ostride OSTRIDE -include "n1f.h") | $(ADD_DATE) | $(INDENT) >$@

sn1fv_%.c:  $(CODELET_DEPS)
	echo "$*" | tr '_' \\n | (read n; read is; read os;	\
        echo "#define ISTRIDE $$is";				\
        echo "#define OSTRIDE $$os";				\
        echo "#define NAME X(codelet_sn1fv_$${n}_$${is}_$${os})";\
        echo "#include \"sn1fv_$${n}.cod\"") >$@

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
