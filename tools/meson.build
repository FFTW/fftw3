wisdom_libs = [fftw_lib, libbench2]
wisdom_deps = [fftw_src.dependencies()]
wisdom_src = files('fftw-wisdom.c')

if threads_dep.found()
    if not conf.get('COMBINED_THREADS')
        wisdom_libs += fftw_threads_lib
        wisdom_deps += threads_dep
    endif
elif openmp_dep.found()
    wisdom_libs += fftw_omp_lib
    wisdom_deps += openmp_dep
endif


executable(
    f'fftw@prec_suffix@-wisdom',
    wisdom_src,
    link_with: wisdom_libs,
    include_directories: '..',
    objects: bench_exe.extract_objects('bench.c', 'fftw-bench.c'),
    dependencies: wisdom_deps,
    install: true,
)

cfgv = {
    'PACKAGE': meson.project_name(),
    'VERSION': meson.project_version(),
    'PREC_SUFFIX': prec_suffix,
}

configure_file(
    configuration: cfgv,
    input: 'fftw-wisdom-to-conf.in',
    output: '@BASENAME@',
    install: true,
    install_dir: get_option('bindir')
)

configure_file(
    configuration: cfgv,
    input: 'fftw_wisdom.1.in',
    output: '@BASENAME@',
    install: true,
    install_dir: get_option('mandir') / 'man1'
)

install_man('fftw-wisdom-to-conf.1')
