genfft_dir=../../genfft
n_sizes="2 4 6 8 10 12 14 16 32"
n_sizes_double="3 5 7 9 11 13 15"
t_sizes="2 3 4 5 6 7 8 9 10 12 15 16 32"

n_flags="-standalone -fma -reorder-insns -simd -compact -variables 100000 -with-ostride 2 -include fftw-spu.h"
t_flags="-standalone -fma -reorder-insns -simd -compact -variables 100000 -include fftw-spu.h -trivial-stores"

indent="indent -kr -cs -i5 -l800 -fca -nfc1 -sc -sob -cli4 -TR -Tplanner -TV"

for n in $n_sizes; do
$genfft_dir/gen_notw_c $n_flags -store-multiple 2 -n $n -name "X(spu_n2fv_$n)" | $indent >spu_n2fv_$n.c
done

for n in $n_sizes_double; do
$genfft_dir/gen_notw_c $n_flags -n $n -name "X(spu_n2fv_$n)" | $indent >spu_n2fv_$n.c
done

for n in $t_sizes; do
$genfft_dir/gen_twiddle_c  $t_flags -n $n -name "X(spu_t1fv_$n)" | $indent >spu_t1fv_$n.c
done
