###########################################################################
# The following lines specify the REDFT/RODFT/DHT sizes for which to generate
# specialized codelets.  Currently, only REDFT01/10 of size 8 (used in JPEG).

# e<a><b>_<n> is a hard-coded REDFT<a><b> FFT (DCT) of size <n>
set(E00 )# e00_2.c e00_3.c e00_4.c e00_5.c e00_6.c e00_7.c e00_8.c
set(E01 e01_8.c) # e01_2.c e01_3.c e01_4.c e01_5.c e01_6.c e01_7.c
set(E10 e10_8.c) # e10_2.c e10_3.c e10_4.c e10_5.c e10_6.c e10_7.c
set(E11 ) # e11_2.c e11_3.c e11_4.c e11_5.c e11_6.c e11_7.c e11_8.c

# o<a><b>_<n> is a hard-coded RODFT<a><b> FFT (DST) of size <n>
set(O00 ) # o00_2.c o00_3.c o00_4.c o00_5.c o00_6.c o00_7.c o00_8.c
set(O01 ) # o01_2.c o01_3.c o01_4.c o01_5.c o01_6.c o01_7.c o01_8.c
set(O10 ) # o10_2.c o10_3.c o10_4.c o10_5.c o10_6.c o10_7.c o10_8.c
set(O11 ) # o11_2.c o11_3.c o11_4.c o11_5.c o11_6.c o11_7.c o11_8.c

# dht_<n> is a hard-coded DHT of size <n>
set(DHT ) # dht_2.c dht_3.c dht_4.c dht_5.c dht_6.c dht_7.c dht_8.c

###########################################################################
set(fftw_rdft_r2r_scalar_codelets
  ${E00} ${E01} ${E10} ${E11} ${O00} ${O01} ${O10} ${O11} ${DHT}
)
