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
n1fv_16.c

# as above, but FFTW_BACKWARD
N1B = n1bv_2.c n1bv_3.c n1bv_4.c n1bv_5.c n1bv_6.c n1bv_7.c n1bv_8.c	\
n1bv_9.c n1bv_10.c n1bv_11.c n1bv_12.c n1bv_13.c n1bv_14.c n1bv_15.c	\
n1bv_16.c

# as above, with restricted input vector stride
N2F = n2fv_2.c n2fv_3.c n2fv_4.c n2fv_5.c n2fv_6.c n2fv_7.c n2fv_8.c	\
n2fv_9.c n2fv_10.c n2fv_11.c n2fv_12.c n2fv_13.c n2fv_14.c n2fv_15.c	\
n2fv_16.c

N2B = n2bv_2.c n2bv_3.c n2bv_4.c n2bv_5.c n2bv_6.c n2bv_7.c n2bv_8.c	\
n2bv_9.c n2bv_10.c n2bv_11.c n2bv_12.c n2bv_13.c n2bv_14.c n2bv_15.c	\
n2bv_16.c

###########################################################################
# m* codelets are like n* codelets, but with the loop body in
# a separate function.  This nonsense is necessary, compilers
# being what they are.
M1F = m1fv_16.c m1fv_32.c m1fv_64.c

M1B = m1bv_16.c m1bv_32.c m1bv_64.c

M2F = m2fv_16.c m2fv_32.c m2fv_64.c

M2B = m2bv_16.c m2bv_32.c m2bv_64.c

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
SIMD_CODELETS = $(N1F) $(N1B) $(N2F) $(N2B) $(M1F) $(M1B) $(M2F)	\
$(M2B) $(T1F) $(T1B) $(Q1F) $(Q1B)

if HAVE_SIMD
ALL_CODELETS = $(SIMD_CODELETS)
else
ALL_CODELETS = 
endif

EXTRA_DIST = $(SIMD_CODELETS)
BUILT_SOURCES= $(ALL_CODELETS) $(CODLIST)

libdft_codelets_simd_la_SOURCES = $(BUILT_SOURCES)

SOLVTAB_NAME = X(solvtab_dft_simd)

# special rules for regenerating codelets.
include $(top_srcdir)/support/Makefile.codelets

if MAINTAINER_MODE
GFLAGS = -simd $(FLAGS_COMMON)

n1fv_%.c:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_C) $(GFLAGS) -n $* -name n1fv_$* -include "n1f.h") | $(ADD_DATE) | $(INDENT) >$@

n2fv_%.c:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_C) $(GFLAGS) -n $* -name n2fv_$* -with-ostride 2 -include "n2f.h") | $(ADD_DATE) | $(INDENT) >$@

n1bv_%.c:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_C) $(GFLAGS) -sign 1 -n $* -name n1bv_$* -include "n1b.h") | $(ADD_DATE) | $(INDENT) >$@

n2bv_%.c:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_C) $(GFLAGS) -sign 1 -n $* -name n2bv_$* -with-ostride 2 -include "n2b.h") | $(ADD_DATE) | $(INDENT) >$@

m1fv_%.c:  $(CODELET_DEPS) $(GEN_NOTW_NOINLINE_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_NOINLINE_C) $(GFLAGS) -n $* -name m1fv_$* -include "n1f.h") | $(ADD_DATE) | $(INDENT) >$@

m2fv_%.c:  $(CODELET_DEPS) $(GEN_NOTW_NOINLINE_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_NOINLINE_C) $(GFLAGS) -n $* -name m2fv_$* -with-ostride 2 -include "n2f.h") | $(ADD_DATE) | $(INDENT) >$@

m1bv_%.c:  $(CODELET_DEPS) $(GEN_NOTW_NOINLINE_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_NOINLINE_C) $(GFLAGS) -sign 1 -n $* -name m1bv_$* -include "n1b.h") | $(ADD_DATE) | $(INDENT) >$@

m2bv_%.c:  $(CODELET_DEPS) $(GEN_NOTW_NOINLINE_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_NOTW_NOINLINE_C) $(GFLAGS) -sign 1 -n $* -name m2bv_$* -with-ostride 2 -include "n2b.h") | $(ADD_DATE) | $(INDENT) >$@

t1fv_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_TWIDDLE_C) $(GFLAGS) -n $* -name t1fv_$* -include "t1f.h") | $(ADD_DATE) | $(INDENT) >$@

t1bv_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_TWIDDLE_C) $(GFLAGS) -n $* -name t1bv_$* -include "t1b.h" -sign 1) | $(ADD_DATE) | $(INDENT) >$@

q1fv_%.c:  $(CODELET_DEPS) $(GEN_TWIDSQ_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_TWIDSQ_C) $(GFLAGS) -n $* -dif -name q1fv_$* -include "q1f.h") | $(ADD_DATE) | $(INDENT) >$@

q1bv_%.c:  $(CODELET_DEPS) $(GEN_TWIDSQ_C)
	($(PRELUDE_COMMANDS_DFT); $(GEN_TWIDSQ_C) $(GFLAGS) -n $* -dif -name q1bv_$* -include "q1b.h" -sign 1) | $(ADD_DATE) | $(INDENT) >$@


endif # MAINTAINER_MODE
