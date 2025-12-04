src_par = files(
    'api.c',
    'conf.c',
    'ct.c',
    'dft-vrank-geq1.c',
    'f77api.c',
    'hc2hc.c',
    'rdft-vrank-geq1.c',
    'vrank-geq1-rdft2.c',
)

threads_dep = dependency('threads', required: get_option('threads'))
conf.set('HAVE_THREADS', threads_dep.found())
conf.set(
    'USING_POSIX_THREADS',
    threads_dep.found() and cc.has_header('pthread.h'),
)

openmp_dep = dependency('openmp', required: get_option('openmp'))
conf.set('HAVE_OPENMP', openmp_dep.found())

combined_threads = get_option('combined_threads').require(
    threads_dep.found(),
    error_message: 'combined_threads: requires threads',
)
conf.set('COMBINED_THREADS', combined_threads.allowed())

fftw_threads_srcset.add([threads_dep, src_par, files('threads.c')])
fftw_omp_srcset.add([openmp_dep, src_par, files('openmp.c')])

fftw_srcset.add_all(when: ['COMBINED_THREADS'], if_true: fftw_threads_srcset)

summary(
    {
        'threads': threads_dep.found(),
        'OpenMP': openmp_dep.found(),
    },
    bool_yn: true,
    section: 'Library'
)
