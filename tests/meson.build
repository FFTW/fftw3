bench_src = files('bench.c', 'fftw-bench.c', 'hook.c')
bench_libs = [fftw_lib, libbench2]
bench_deps = [fftw_src.dependencies()]
bench_defs = []

if threads_dep.found()
    if not conf.get('COMBINED_THREADS')
        bench_libs += fftw_threads_lib
        bench_deps += threads_dep
    endif
elif openmp_dep.found()
    bench_libs += fftw_omp_lib
    bench_deps += openmp_dep
endif

bench_exe = executable(
    'bench',
    bench_src,
    build_by_default: false,
    c_args: bench_defs,
    link_with: bench_libs,
    include_directories: fftw_root_inc,
    dependencies: bench_deps,
)

perl = find_program('perl', required: false)
summary('Tests', perl.found(), bool_yn: true, section: 'Build')

if not perl.found()
    subdir_done()
endif
check_pl = ['-w', meson.current_source_dir() / 'check.pl', '-v', bench_exe]

test(
    'random, count=30',
    perl,
    suite: ['basic'],
    timeout: 0,
    is_parallel: true,
    args: [check_pl, '-r', '-c=30'],
)
test(
    'random, count=30, nthreads=2',
    perl,
    suite: ['basic', 'threaded'],
    timeout: 0,
    is_parallel: false,
    args: [check_pl, '-r', '-c=30', '--nthreads=2'],
)
test(
    'random, count=5, nthreads=2, threads_callback',
    perl,
    suite: ['basic', 'threaded'],
    timeout: 0,
    is_parallel: false,
    args: [check_pl, '-r', '-c=5', '--nthreads=2', '--threads_callback'],
)

test(
    'all, validate wisdom',
    perl,
    suite: ['big'],
    timeout: 0,
    is_parallel: true,
    args: [check_pl, '-a', '--validate-wisdom'],
)
test(
    'all, validate wisdom, nthreads=2',
    perl,
    suite: ['big', 'threaded'],
    timeout: 0,
    is_parallel: false,
    args: [check_pl, '-a', '--validate-wisdom', '--nthreads=2'],
)
test(
    'all, validate wisdom, nthreads=3',
    perl,
    suite: ['big', 'threaded'],
    timeout: 0,
    is_parallel: false,
    args: [check_pl, '-a', '--validate-wisdom', '--nthreads=3'],
)
test(
    'all, validate wisdom, nthreads=10',
    perl,
    suite: ['big', 'threaded'],
    timeout: 0,
    is_parallel: false,
    args: [check_pl, '-a', '--validate-wisdom', '--nthreads=10'],
)
test(
    'all, validate wisdom, nthreads=2, threads_callback',
    perl,
    suite: ['big', 'threaded'],
    timeout: 0,
    is_parallel: false,
    args: [
        check_pl,
        '-a',
        '--validate-wisdom',
        '--nthreads=2',
        '--threads_callback',
    ],
)

test(
    'random, count=1',
    perl,
    suite: ['small'],
    timeout: 0,
    is_parallel: true,
    args: [check_pl, '-r', '-c=1'],
)
test(
    'random, count=5, estimate',
    perl,
    suite: ['small'],
    timeout: 0,
    is_parallel: true,
    args: [check_pl, '-r', '-c=5', '--estimate'],
)
test(
    'random, count=2, estimate, nthreads=2',
    perl,
    suite: ['small', 'threaded'],
    timeout: 0,
    is_parallel: false,
    args: [check_pl, '-r', '-c=2', '--estimate', '--nthreads=2'],
)

test(
    'all, patient, paranoid',
    perl,
    suite: ['paranoid'],
    timeout: 0,
    is_parallel: true,
    args: [check_pl, '-a', '--patient', '--paranoid'],
)
foreach n : [10, 7, 3, 2]
    test(
        f'all, patient, paranoid, nthreads=@n@',
        perl,
        suite: ['paranoid', 'threaded'],
        timeout: 0,
        is_parallel: false,
        args: [check_pl, '-a', '--patient', '--paranoid', '--nthreads=@n@'],
    )
endforeach

test(
    'all, exhaustive, paranoid',
    perl,
    suite: ['exhaustive', 'paranoid'],
    timeout: 0,
    is_parallel: true,
    args: [check_pl, '-a', '--exhaustive', '--paranoid'],
)
foreach n : [10, 7, 3, 2]
    test(
        f'all, exhaustive, paranoid, nthreads=@n@',
        perl,
        suite: ['exhaustive', 'paranoid', 'threaded'],
        timeout: 0,
        is_parallel: false,
        args: [check_pl, '-a', '--exhaustive', '--paranoid', '--nthreads=@n@'],
    )
endforeach
test(
    'all, exhaustive, paranoid, nthreads=2, threads_callback',
    perl,
    suite: ['exhaustive', 'paranoid', 'threaded'],
    timeout: 0,
    is_parallel: false,
    args: [
        check_pl,
        '-a',
        '--exhaustive',
        '--paranoid',
        '--nthreads=2',
        '--threads_callback',
    ],
)
