###########################################################################
# n1_<n> is a hard-coded FFT of size <n> (base cases of FFT recursion)
set(N1_code
    n1_2.c n1_3.c n1_4.c n1_5.c n1_6.c n1_7.c n1_8.c n1_9.c n1_10.c
    n1_11.c n1_12.c n1_13.c n1_14.c n1_15.c n1_16.c n1_32.c n1_64.c
    n1_20.c n1_25.c
)
# n1_30.c n1_40.c n1_50.c

###########################################################################
# t1_<r> is a "twiddle" FFT of size <r>, implementing a radix-r DIT step
set(T1_code
    t1_2.c t1_3.c t1_4.c t1_5.c t1_6.c t1_7.c t1_8.c t1_9.c
    t1_10.c t1_12.c t1_15.c t1_16.c t1_32.c t1_64.c
    t1_20.c t1_25.c
    )
 # t1_30.c t1_40.c t1_50.c

# t2_<r> is also a twiddle FFT, but instead of using a complete lookup table
# of trig. functions, it partially generates the trig. values on the fly
# (this is faster for large sizes).
set(T2_code
    t2_4.c t2_8.c t2_16.c t2_32.c t2_64.c
    t2_5.c t2_10.c t2_20.c t2_25.c
    )

###########################################################################
# The F (DIF) codelets are used for a kind of in-place transform algorithm,
# but the planner seems to never (or hardly ever) use them on the machines
# we have access to, preferring the Q codelets and the use of buffers
# for sub-transforms.  So, we comment them out, at least for now.

# f1_<r> is a "twiddle" FFT of size <r>, implementing a radix-r DIF step
#F1 = # f1_2.c f1_3.c f1_4.c f1_5.c f1_6.c f1_7.c f1_8.c f1_9.c f1_10.c f1_12.c f1_15.c f1_16.c f1_32.c f1_64.c

# like f1, but partially generates its trig. table on the fly
#F2 = # f2_4.c f2_8.c f2_16.c f2_32.c f2_64.c

###########################################################################
# q1_<r> is <r> twiddle FFTs of size <r> (DIF step), where the output is
# transposed.  This is used for in-place transposes in sizes that are
# divisible by <r>^2.  These codelets have size ~ <r>^2, so you should
# probably not use <r> bigger than 8 or so.
set(Q1_code q1_2.c q1_4.c q1_8.c  q1_3.c q1_5.c q1_6.c)

###########################################################################
set(fftw_dft_scalar_codelets ${N1_code} ${T1_code} ${T2_code} ${Q1_code})
