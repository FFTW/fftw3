set(HC2CFDFTV hc2cfdftv_2.c hc2cfdftv_4.c hc2cfdftv_6.c hc2cfdftv_8.c
hc2cfdftv_10.c hc2cfdftv_12.c hc2cfdftv_16.c hc2cfdftv_32.c
hc2cfdftv_20.c)

set(HC2CBDFTV hc2cbdftv_2.c hc2cbdftv_4.c hc2cbdftv_6.c hc2cbdftv_8.c
hc2cbdftv_10.c hc2cbdftv_12.c hc2cbdftv_16.c hc2cbdftv_32.c
hc2cbdftv_20.c)

###########################################################################
set(fftw_rdft_simd_codelets ${HC2CFDFTV} ${HC2CBDFTV})
