###########################################################################
# n1fv_<n> is a hard-coded FFTW_FORWARD FFT of size <n>, using SIMD
set(FFTW_dft_simd_N1F n1fv_2.c n1fv_3.c n1fv_4.c n1fv_5.c n1fv_6.c n1fv_7.c n1fv_8.c
n1fv_9.c n1fv_10.c n1fv_11.c n1fv_12.c n1fv_13.c n1fv_14.c n1fv_15.c
n1fv_16.c n1fv_32.c n1fv_64.c n1fv_128.c n1fv_20.c n1fv_25.c)

# as above, with restricted input vector stride
set(FFTW_dft_simd_N2F n2fv_2.c n2fv_4.c n2fv_6.c n2fv_8.c n2fv_10.c n2fv_12.c
n2fv_14.c n2fv_16.c n2fv_32.c n2fv_64.c n2fv_20.c)

# as above, but FFTW_BACKWARD
set(FFTW_dft_simd_N1B n1bv_2.c n1bv_3.c n1bv_4.c n1bv_5.c n1bv_6.c n1bv_7.c n1bv_8.c
n1bv_9.c n1bv_10.c n1bv_11.c n1bv_12.c n1bv_13.c n1bv_14.c n1bv_15.c
n1bv_16.c n1bv_32.c n1bv_64.c n1bv_128.c n1bv_20.c n1bv_25.c)

set(FFTW_dft_simd_N2B n2bv_2.c n2bv_4.c n2bv_6.c n2bv_8.c n2bv_10.c n2bv_12.c
n2bv_14.c n2bv_16.c n2bv_32.c n2bv_64.c n2bv_20.c)

# split-complex codelets
set(FFTW_dft_simd_N2S n2sv_4.c n2sv_8.c n2sv_16.c n2sv_32.c n2sv_64.c)

###########################################################################
# t1fv_<r> is a "twiddle" FFT of size <r>, implementing a radix-r DIT step
# for an FFTW_FORWARD transform, using SIMD
set(FFTW_dft_simd_T1F t1fv_2.c t1fv_3.c t1fv_4.c t1fv_5.c t1fv_6.c t1fv_7.c t1fv_8.c
t1fv_9.c t1fv_10.c t1fv_12.c t1fv_15.c t1fv_16.c t1fv_32.c t1fv_64.c
t1fv_20.c t1fv_25.c)

# same as t1fv_*, but with different twiddle storage scheme
set(FFTW_dft_simd_T2F t2fv_2.c t2fv_4.c t2fv_8.c t2fv_16.c t2fv_32.c t2fv_64.c
    t2fv_5.c t2fv_10.c t2fv_20.c t2fv_25.c)

set(FFTW_dft_simd_T3F t3fv_4.c t3fv_8.c t3fv_16.c t3fv_32.c t3fv_5.c t3fv_10.c
t3fv_20.c t3fv_25.c)

set(FFTW_dft_simd_T1FU t1fuv_2.c t1fuv_3.c t1fuv_4.c t1fuv_5.c t1fuv_6.c t1fuv_7.c
t1fuv_8.c t1fuv_9.c t1fuv_10.c)

# as above, but FFTW_BACKWARD
set(FFTW_dft_simd_T1B t1bv_2.c t1bv_3.c t1bv_4.c t1bv_5.c t1bv_6.c t1bv_7.c t1bv_8.c
t1bv_9.c t1bv_10.c t1bv_12.c t1bv_15.c t1bv_16.c t1bv_32.c t1bv_64.c
t1bv_20.c t1bv_25.c)

# same as t1bv_*, but with different twiddle storage scheme
set(FFTW_dft_simd_T2B t2bv_2.c t2bv_4.c t2bv_8.c t2bv_16.c t2bv_32.c t2bv_64.c
t2bv_5.c t2bv_10.c t2bv_20.c t2bv_25.c)
set(FFTW_dft_simd_T3B t3bv_4.c t3bv_8.c t3bv_16.c t3bv_32.c t3bv_5.c t3bv_10.c
t3bv_20.c t3bv_25.c)
set(FFTW_dft_simd_T1BU t1buv_2.c t1buv_3.c t1buv_4.c t1buv_5.c t1buv_6.c t1buv_7.c
t1buv_8.c t1buv_9.c t1buv_10.c)

# split-complex codelets
set(FFTW_dft_simd_T1S t1sv_2.c t1sv_4.c t1sv_8.c t1sv_16.c t1sv_32.c)
set(FFTW_dft_simd_T2S t2sv_4.c t2sv_8.c t2sv_16.c t2sv_32.c)

###########################################################################
# q1fv_<r> is <r> twiddle FFTW_FORWARD FFTs of size <r> (DIF step),
# where the output is transposed, using SIMD.  This is used for
# in-place transposes in sizes that are divisible by <r>^2.  These
# codelets have size ~ <r>^2, so you should probably not use <r>
# bigger than 8 or so.
set(FFTW_dft_simd_Q1F q1fv_2.c q1fv_4.c q1fv_5.c q1fv_8.c)

# as above, but FFTW_BACKWARD
set(FFTW_dft_simd_Q1B q1bv_2.c q1bv_4.c q1bv_5.c q1bv_8.c)

###########################################################################
set(FFTW_dft_simd_codelets
  ${FFTW_dft_simd_N1F}
  ${FFTW_dft_simd_N1B}
  ${FFTW_dft_simd_N2F}
  ${FFTW_dft_simd_N2B}
  ${FFTW_dft_simd_N2S}
  ${FFTW_dft_simd_T1FU}
  ${FFTW_dft_simd_T1F}
  ${FFTW_dft_simd_T2F}
  ${FFTW_dft_simd_T3F}
  ${FFTW_dft_simd_T1BU}
  ${FFTW_dft_simd_T1B}
  ${FFTW_dft_simd_T2B}
  ${FFTW_dft_simd_T3B}
  ${FFTW_dft_simd_T1S}
  ${FFTW_dft_simd_T2S}
  ${FFTW_dft_simd_Q1F}
  ${FFTW_dft_simd_Q1B}
)
