mpi_opt = get_option('mpi')
mpi_opt = mpi_opt.require(
    precision != 'quad',
    error_message: 'quad precision is not supported in MPI',
)
mpi_dep = dependency('mpi', language: 'c', required: mpi_opt)
summary('MPI', mpi_dep.found(), bool_yn: true, section: 'Library')

if mpi_dep.found()
    fftw_mpi_srcset.add(mpi_dep)

    if not fs.is_file('f03-wrap.c')
        preprocessed_h = cc.preprocess(
            # Meson is being a little too smart here and won’t pass .h files.
            # Still prefer this approach over explicitly calling CPP
            fs.copyfile('fftw3-mpi.h', 'fftw3-mpi.c'),
            dependencies: mpi_dep,
            include_directories: fftw_api_inc,
            output: '@BASENAME@.preproc.h',
        )

        f03_wrap = custom_target(
            input: preprocessed_h,
            output: 'f03-wrap.c',
            capture: true,
            command: [
                meson.current_source_dir() / 'f03-wrap-meson.sh',
                meson.current_source_dir() / 'genf03-wrap.pl',
                '@INPUT@',
            ],
        )
    else
        f03_wrap = files('f03-wrap.c')
    endif

    fftw_extra_srcset.add(f03_wrap)

    mpi_transpose_src = files(
        'transpose-alltoall.c',
        'transpose-pairwise.c',
        'transpose-problem.c',
        'transpose-recurse.c',
        'transpose-solve.c',
    )

    mpi_dft_src = files(
        'dft-problem.c',
        'dft-rank-geq2-transposed.c',
        'dft-rank-geq2.c',
        'dft-rank1-bigvec.c',
        'dft-rank1.c',
        'dft-serial.c',
        'dft-solve.c',
    )

    mpi_rdft_src = files(
        'rdft-problem.c',
        'rdft-rank-geq2-transposed.c',
        'rdft-rank-geq2.c',
        'rdft-rank1-bigvec.c',
        'rdft-serial.c',
        'rdft-solve.c',
    )

    mpi_rdft2_src = files(
        'rdft2-problem.c',
        'rdft2-rank-geq2-transposed.c',
        'rdft2-rank-geq2.c',
        'rdft2-serial.c',
        'rdft2-solve.c',
    )

    mpi_src = files(
        'any-true.c',
        'api.c',
        'block.c',
        'choose-radix.c',
        'conf.c',
        'dtensor.c',
        'rearrange.c',
        'wisdom-api.c',
    )

    fftw_mpi_srcset.add(
        [
            mpi_transpose_src,
            mpi_dft_src,
            mpi_rdft_src,
            mpi_rdft2_src,
            mpi_src,
            f03_wrap,
        ],
    )

    fftw_mpi_src = fftw_mpi_srcset.apply(conf)
    fftw_mpi_lib = library(
        fftw_lib_name + '_mpi',
        fftw_mpi_src.sources(),
        c_shared_args: shlib_args,
        link_with: fftw_lib,
        dependencies: fftw_mpi_src.dependencies(),
        version: shlib_version,
        include_directories: fftw_root_inc,
        install: true,
    )

    fftw3_mpi_dep = declare_dependency(
        link_with: fftw_mpi_lib,
        include_directories: inc,
    )
    pkg.generate(
        fftw_mpi_lib,
        filebase: fftw_mpi_lib.name(),
        name: pcname,
        description: pcdesc,
        url: pcurl,
    )

    mpirun = find_program('mpirun', required: false)

    if mpirun.found()
        mpi_bench_libs = [fftw_lib, fftw_mpi_lib, libbench2]
        mpi_bench_deps = [fftw_src.dependencies(), fftw_mpi_src.dependencies()]
        mpi_bench_defs = []

        if threads_dep.found()
            if not conf.get('COMBINED_THREADS')
                mpi_bench_libs += fftw_threads_lib
                mpi_bench_deps += threads_dep
            endif
        elif openmp_dep.found()
            mpi_bench_libs += fftw_omp_lib
            mpi_bench_deps += openmp_dep
        endif

        mpi_bench_src = files(
            meson.project_source_root() / 'tests/fftw-bench.c',
            meson.project_source_root() / 'tests/hook.c',
            'mpi-bench.c',
        )

        mpi_bench_exe = executable(
            'mpi-bench',
            mpi_bench_src,
            build_by_default: false,
            c_args: mpi_bench_defs,
            link_with: mpi_bench_libs,
            include_directories: fftw_root_inc,
            dependencies: mpi_bench_deps,
        )
    else
        warning('mpirun not found, skipping MPI tests')
    endif

    summary('MPI Tests', mpirun.found(), bool_yn: true, section: 'Build')
endif
