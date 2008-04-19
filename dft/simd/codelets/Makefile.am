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
AM_CFLAGS = $(SIMD_CFLAGS)
noinst_LTLIBRARIES = libdft_simd_codelets.la

###########################################################################
# n1fv_<n> is a hard-coded FFTW_FORWARD FFT of size <n>, using SIMD
N1F = n1fv_2.c n1fv_3.c n1fv_4.c n1fv_5.c n1fv_6.c n1fv_7.c n1fv_8.c	\
n1fv_9.c n1fv_10.c n1fv_11.c n1fv_12.c n1fv_13.c n1fv_14.c n1fv_15.c	\
n1fv_16.c n1fv_32.c n1fv_64.c \
n1fv_20.c n1fv_25.c # n1fv_30.c n1fv_40.c n1fv_50.c

# as above, with restricted input vector stride
N2F = n2fv_2.c n2fv_4.c n2fv_6.c n2fv_8.c n2fv_10.c n2fv_12.c	\
n2fv_14.c n2fv_16.c n2fv_32.c n2fv_64.c \
n2fv_20.c # n2fv_30.c n2fv_40.c n2fv_50.c

# as above, but FFTW_BACKWARD
N1B = n1bv_2.c n1bv_3.c n1bv_4.c n1bv_5.c n1bv_6.c n1bv_7.c n1bv_8.c	\
n1bv_9.c n1bv_10.c n1bv_11.c n1bv_12.c n1bv_13.c n1bv_14.c n1bv_15.c	\
n1bv_16.c n1bv_32.c n1bv_64.c \
n1bv_20.c n1bv_25.c # n1bv_30.c n1bv_40.c n1bv_50.c

N2B = n2bv_2.c n2bv_4.c n2bv_6.c n2bv_8.c n2bv_10.c n2bv_12.c	\
n2bv_14.c n2bv_16.c n2bv_32.c n2bv_64.c \
n2bv_20.c # n2bv_30.c n2bv_40.c n2bv_50.c

# split-complex codelets 
N2S = n2sv_4.c n2sv_8.c n2sv_16.c n2sv_32.c n2sv_64.c

###########################################################################
# t1fv_<r> is a "twiddle" FFT of size <r>, implementing a radix-r DIT step
# for an FFTW_FORWARD transform, using SIMD
T1F = t1fv_2.c t1fv_3.c t1fv_4.c t1fv_5.c t1fv_6.c t1fv_7.c t1fv_8.c	\
t1fv_9.c t1fv_10.c t1fv_12.c t1fv_15.c t1fv_16.c t1fv_32.c t1fv_64.c \
t1fv_20.c t1fv_25.c # t1fv_30.c t1fv_40.c t1fv_50.c

# same as t1fv_*, but with different twiddle storage scheme
T2F = t2fv_2.c t2fv_4.c t2fv_8.c t2fv_16.c t2fv_32.c t2fv_64.c \
t2fv_5.c t2fv_10.c t2fv_20.c t2fv_25.c
T3F = t3fv_4.c t3fv_8.c t3fv_16.c t3fv_32.c \
t3fv_5.c t3fv_10.c t3fv_20.c t3fv_25.c
T1FU = t1fuv_2.c t1fuv_3.c t1fuv_4.c t1fuv_5.c t1fuv_6.c t1fuv_7.c	\
t1fuv_8.c t1fuv_9.c t1fuv_10.c

# as above, but FFTW_BACKWARD
T1B = t1bv_2.c t1bv_3.c t1bv_4.c t1bv_5.c t1bv_6.c t1bv_7.c t1bv_8.c	\
t1bv_9.c t1bv_10.c t1bv_12.c t1bv_15.c t1bv_16.c t1bv_32.c t1bv_64.c \
t1bv_20.c t1bv_25.c # t1bv_30.c t1bv_40.c t1bv_50.c

# same as t1bv_*, but with different twiddle storage scheme
T2B = t2bv_2.c t2bv_4.c t2bv_8.c t2bv_16.c t2bv_32.c t2bv_64.c \
t2bv_5.c t2bv_10.c t2bv_20.c t2bv_25.c
T3B = t3bv_4.c t3bv_8.c t3bv_16.c t3bv_32.c \
t3bv_5.c t3bv_10.c t3bv_20.c t3bv_25.c
T1BU = t1buv_2.c t1buv_3.c t1buv_4.c t1buv_5.c t1buv_6.c t1buv_7.c	\
t1buv_8.c t1buv_9.c t1buv_10.c

# split-complex codelets
T1S = t1sv_2.c t1sv_4.c t1sv_8.c t1sv_16.c t1sv_32.c #t1sv_64.c
T2S = t2sv_4.c t2sv_8.c t2sv_16.c t2sv_32.c #t2sv_64.c

###########################################################################
# q1fv_<r> is <r> twiddle FFTW_FORWARD FFTs of size <r> (DIF step),
# where the output is transposed, using SIMD.  This is used for
# in-place transposes in sizes that are divisible by <r>^2.  These
# codelets have size ~ <r>^2, so you should probably not use <r>
# bigger than 8 or so.
Q1F = q1fv_2.c q1fv_4.c q1fv_5.c q1fv_8.c 

# as above, but FFTW_BACKWARD
Q1B = q1bv_2.c q1bv_4.c q1bv_5.c q1bv_8.c

###########################################################################
SIMD_CODELETS = $(N1F) $(N1B) $(N2F) $(N2B) $(N2S) $(T1FU) $(T1F)	\
$(T2F) $(T3F) $(T1BU) $(T1B) $(T2B) $(T3B) $(T1S) $(T2S) $(Q1F) $(Q1B)

if HAVE_SIMD
ALL_CODELETS = $(SIMD_CODELETS)
else
ALL_CODELETS = 
endif

EXTRA_DIST = $(SIMD_CODELETS)
BUILT_SOURCES= $(ALL_CODELETS) $(CODLIST)

libdft_simd_codelets_la_SOURCES = $(BUILT_SOURCES)

SOLVTAB_NAME = X(solvtab_dft_simd)

# special rules for regenerating codelets.
include $(top_srcdir)/support/Makefile.codelets

if MAINTAINER_MODE
GFLAGS = -simd $(FLAGS_COMMON) -pipeline-latency 8
FLAGS_T2S=-twiddle-log3 -precompute-twiddles
FLAGS_T3=-twiddle-log3 -precompute-twiddles -no-generate-bytw

n1fv_%.c:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS_DFT); $(TWOVERS) $(GEN_NOTW_C) $(GFLAGS) -n $* -name n1fv_$* -include "n1f.h") | $(ADD_DATE) | $(INDENT) >$@

n2fv_%.c:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS_DFT); $(TWOVERS) $(GEN_NOTW_C) $(GFLAGS) -n $* -name n2fv_$* -with-ostride 2 -include "n2f.h" -store-multiple 2) | $(ADD_DATE) | $(INDENT) >$@

n1bv_%.c:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS_DFT); $(TWOVERS) $(GEN_NOTW_C) $(GFLAGS) -sign 1 -n $* -name n1bv_$* -include "n1b.h") | $(ADD_DATE) | $(INDENT) >$@

n2bv_%.c:  $(CODELET_DEPS) $(GEN_NOTW_C)
	($(PRELUDE_COMMANDS_DFT); $(TWOVERS) $(GEN_NOTW_C) $(GFLAGS) -sign 1 -n $* -name n2bv_$* -with-ostride 2 -include "n2b.h"  -store-multiple 2) | $(ADD_DATE) | $(INDENT) >$@

n2sv_%.c:  $(CODELET_DEPS) $(GEN_NOTW)
	($(PRELUDE_COMMANDS_DFT); $(TWOVERS) $(GEN_NOTW) $(GFLAGS) -n $* -name n2sv_$* -with-ostride 1 -include "n2s.h" -store-multiple 4) | $(ADD_DATE) | $(INDENT) >$@

t1fv_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE_C)
	($(PRELUDE_COMMANDS_DFT); $(TWOVERS) $(GEN_TWIDDLE_C) $(GFLAGS) -n $* -name t1fv_$* -include "t1f.h") | $(ADD_DATE) | $(INDENT) >$@

t1fuv_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE_C)
	($(PRELUDE_COMMANDS_DFT); $(TWOVERS) $(GEN_TWIDDLE_C) $(GFLAGS) -n $* -name t1fuv_$* -include "t1fu.h") | $(ADD_DATE) | $(INDENT) >$@

t2fv_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE_C)
	($(PRELUDE_COMMANDS_DFT); $(TWOVERS) $(GEN_TWIDDLE_C) $(GFLAGS) -n $* -name t2fv_$* -include "t2f.h") | $(ADD_DATE) | $(INDENT) >$@

t3fv_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE_C)
	($(PRELUDE_COMMANDS_DFT); $(TWOVERS) $(GEN_TWIDDLE_C) $(GFLAGS) $(FLAGS_T3) -n $* -name t3fv_$* -include "t3f.h") | $(ADD_DATE) | $(INDENT) >$@

t1bv_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE_C)
	($(PRELUDE_COMMANDS_DFT); $(TWOVERS) $(GEN_TWIDDLE_C) $(GFLAGS) -n $* -name t1bv_$* -include "t1b.h" -sign 1) | $(ADD_DATE) | $(INDENT) >$@

t1buv_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE_C)
	($(PRELUDE_COMMANDS_DFT); $(TWOVERS) $(GEN_TWIDDLE_C) $(GFLAGS) -n $* -name t1buv_$* -include "t1bu.h" -sign 1) | $(ADD_DATE) | $(INDENT) >$@

t2bv_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE_C)
	($(PRELUDE_COMMANDS_DFT); $(TWOVERS) $(GEN_TWIDDLE_C) $(GFLAGS) -n $* -name t2bv_$* -include "t2b.h" -sign 1) | $(ADD_DATE) | $(INDENT) >$@

t3bv_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE_C)
	($(PRELUDE_COMMANDS_DFT); $(TWOVERS) $(GEN_TWIDDLE_C) $(GFLAGS) $(FLAGS_T3) -n $* -name t3bv_$* -include "t3b.h" -sign 1) | $(ADD_DATE) | $(INDENT) >$@

t1sv_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS_DFT); $(TWOVERS) $(GEN_TWIDDLE) $(GFLAGS) -n $* -name t1sv_$* -include "ts.h") | $(ADD_DATE) | $(INDENT) >$@

t2sv_%.c:  $(CODELET_DEPS) $(GEN_TWIDDLE)
	($(PRELUDE_COMMANDS_DFT); $(TWOVERS) $(GEN_TWIDDLE) $(GFLAGS) $(FLAGS_T2S) -n $* -name t2sv_$* -include "ts.h") | $(ADD_DATE) | $(INDENT) >$@

q1fv_%.c:  $(CODELET_DEPS) $(GEN_TWIDSQ_C)
	($(PRELUDE_COMMANDS_DFT); $(TWOVERS) $(GEN_TWIDSQ_C) $(GFLAGS) -n $* -dif -name q1fv_$* -include "q1f.h") | $(ADD_DATE) | $(INDENT) >$@

q1bv_%.c:  $(CODELET_DEPS) $(GEN_TWIDSQ_C)
	($(PRELUDE_COMMANDS_DFT); $(TWOVERS) $(GEN_TWIDSQ_C) $(GFLAGS) -n $* -dif -name q1bv_$* -include "q1b.h" -sign 1) | $(ADD_DATE) | $(INDENT) >$@


endif # MAINTAINER_MODE
