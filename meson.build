project(
    'fftw',
    'c',
    meson_version: '>=1.3.0',
    # version will be cemented automatically for dist tarballs
    version: run_command(
        'git',
        '-C', meson.project_source_root(),
        'describe', '--tags', '--always', '--dirty',
        check: true,
    ).stdout().strip().replace('fftw-', '',),
    license: 'GPL-2.0',
    license_files: 'COPYING',
    default_options: {'buildtype': 'release'},
)

fs = import('fs')

is_git = fs.exists('.git')
version_clean = meson.project_version().split('-')[0].split('.')

# match libtool versioning scheme
so_current = 9  # increment for all API changes
so_revision = version_clean[2].to_int()  # always increment
so_age = 6  # increment only for backwards-compatible API changes
shlib_version = '@0@.@1@.@2@'.format(so_current - so_age, so_age, so_revision)

cc = meson.get_compiler('c')
cc_syntax = cc.get_argument_syntax()
cc_id = cc.get_id()
cc_str = ' '.join(cc.cmd_array() + get_option('c_args'))

# We’re not exporting some internal symbols that are needed to link extra libs
# and tools to the main library, so MSVC-style linkers only work for static libs
if ['link', 'lld-link', 'xilink'].contains(cc.get_linker_id())
    if get_option('default_library') != 'static'
        error('Only static libraries are supported with this linker. Change with: --default-library static')
    endif
endif

fftw_root_inc = include_directories('.')
support_dir = meson.project_source_root() / 'support'

add_global_arguments(cc.get_supported_arguments('-Wno-unused'), language: 'c')

if cc_syntax == 'gcc'
    cc_def = '-D'
elif cc_syntax == 'msvc'
    cc_def = '/D'
endif

shlib_args = cc_def + 'FFTW_DLL'

conf = configuration_data(
    {'C_COMPILER': cc_str, 'DISABLE_FORTRAN': not get_option('fortran')},
)
conf.set_quoted('PACKAGE', meson.project_name())
conf.set_quoted('PACKAGE_VERSION', meson.project_version())
conf.set_quoted('VERSION', meson.project_version())
conf.set_quoted('FFTW_CC', cc_str)

check_headers = [
    'alloca.h',
    'altivec.h',
    'c_asm.h',
    'dlfcn.h',
    'intrinsics.h',
    'inttypes.h',
    'libintl.h',
    'limits.h',
    'mach/mach_time.h',
    'malloc.h',
    'memory.h',
    'stddef.h',
    'stdint.h',
    'stdlib.h',
    'string.h',
    'strings.h',
    'sys/types.h',
    'sys/time.h',
    'sys/stat.h',
    'sys/sysctl.h',
    'time.h',
    'uintptr.h',
    'unistd.h',
    'xmmintrin.h',
]

foreach h : check_headers
    if cc.has_header(h)
        conf.set10('HAVE_' + h.underscorify().to_upper(), true)
    endif
endforeach

conf.set10(
    'TIME_WITH_SYS_TIME',
    conf.has('HAVE_TIME_H') and conf.has('HAVE_SYS_TIME_H'),
)

check_decls = {
    'stdlib.h': ['drand48', 'srand48', 'posix_memalign'],
    'math.h': ['cosl', 'sinl'],
    'malloc.h': ['memalign'],
}

foreach h, decls : check_decls
    foreach d : decls
        conf.set10(
            'HAVE_DECL_' + d.underscorify().to_upper(),
            cc.has_header_symbol(h, d),
        )
    endforeach
endforeach

check_funcs = [
    'BSDgettimeofday',
    'gettimeofday',
    'hrtime_t',
    'getpagesize',
    'sysctl',
    'drand48',
    'srand48',
    'posix_memalign',
    'memalign',
    'mach_absolute_time',
    'isnan',
    'cosl',
    'sinl',
    'snprintf',
    'strchr',
]

foreach f : check_funcs
    if cc.has_function(f)
        conf.set('HAVE_' + f.underscorify().to_upper(), 1)
    endif
endforeach

conf.set10('HAVE_CLOCK_GETTIME', cc.has_header_symbol('time.h', 'clock_gettime'))
conf.set(
    'HAVE_ALLOCA',
    cc.has_header_symbol('alloca.h', 'alloca') or cc.has_header_symbol(
        'malloc.h',
        'alloca',
    ),
)

ssmod = import('sourceset')
fftw_srcset = ssmod.source_set()
fftw_threads_srcset = ssmod.source_set()
fftw_omp_srcset = ssmod.source_set()
fftw_mpi_srcset = ssmod.source_set()
fftw_extra_srcset = ssmod.source_set()

m_dep = cc.find_library('m', required: false)
conf.set('HAVE_LIBM', m_dep.found())
fftw_srcset.add(m_dep)

sizes = [
    'float',
    'double',
    'int',
    'long',
    'long long',
    'unsigned int',
    'unsigned long',
    'unsigned long long',
    'size_t',
    'ptrdiff_t',
]

conf.set('FFTW_RANDOM_ESTIMATOR', get_option('random_estimator'))
conf.set('FFTW_ENABLE_ALLOCA', get_option('alloca'))
conf.set('WITH_SLOW_TIMER', get_option('slow_timer'))
conf.set(
    'HAVE_MIPS_ZBUS_TIMER',
    get_option('mips_zbus_timer').require(
        host_machine.cpu_family() in ['mips', 'mips64'],
        error_message: 'Only makes sense when targeting MIPS',
    ).allowed(),
)

have_memalign = (
    conf.has('HAVE_MEMALIGN') or
    conf.has('HAVE_POSIX_MEMALIGN')
)
opt_our_malloc = get_option('our_malloc')
opt_our_malloc = opt_our_malloc.enable_auto_if(not have_memalign)
conf.set('WITH_OUR_MALLOC', opt_our_malloc.allowed())

prefer_fma = get_option('prefer_fma')
prefer_fma = prefer_fma.disable_auto_if(
    host_machine.cpu_family() not in ['ppc', 'ppc64', 'ia64', 'mips64'],
)
conf.set('ARCH_PREFERS_FMA', prefer_fma.allowed())

incoming_stack_boundary = get_option('incoming_stack_boundary')
if incoming_stack_boundary > 0
    isb_arg = f'-mincoming-stack-boundary=@incoming_stack_boundary@'
    if cc_syntax == 'gcc' and cc.has_argument(isb_arg)
        add_global_arguments(isb_arg, language: 'c')
    else
        error('incoming_stack_boundary: not available with this compiler')
    endif
endif

foreach t : sizes
    conf.set('SIZEOF_' + t.underscorify().to_upper(), cc.sizeof(t))
endforeach
conf.set('SIZEOF_VOID_P', cc.sizeof('void*'))
conf.set(
    'C_FFTW_R2R_KIND',
    'C_INT@0@'.format(
        cc.sizeof(
            'fftw_r2r_kind',
            prefix: '#include "api/fftw3.h"',
            include_directories: fftw_root_inc,
        ) * 8,
    ),
)

precision = get_option('precision')
if precision == 'single'
    prec_suffix = 'f'
    conf.set10('FFTW_SINGLE', true)
    conf.set10('BENCHFFT_SINGLE', true)
elif precision == 'long-double'
    prec_suffix = 'l'
    conf.set10('FFTW_LDOUBLE', true)
    conf.set10('BENCHFFT_LDOUBLE', true)
elif precision == 'quad'
    prec_suffix = 'q'
    conf.set10('FFTW_QUAD', true)
    conf.set10('BENCHFFT_QUAD', true)
else
    prec_suffix = ''
endif

fftw_lib_name = 'fftw3' + prec_suffix

quadmath_dep = cc.find_library('quadmath', required: precision == 'quad')
conf.set10('HAVE_LIBQUADMATH', quadmath_dep.found())
fftw_srcset.add(quadmath_dep)

simd_checks = {
    # flags: extra compiler flags for compilers using a specific syntax.
    # meson supports gcc-like and msvc-like compilers.
    'sse2': {
        'flags': {
            'gcc': precision == 'single' ? ['-msse'] : ['-msse2'],
            'msvc': precision == 'single' ? ['/arch:SSE'] : ['/arch:SSE2'],
        },
        'flags_global': {'gcc': ['-mfpmath=sse']},
        'precision': ['single', 'double'],
    },
    'avx': {
        'flags': {'gcc': ['-mavx'], 'msvc': ['/arch:AVX']},
        'precision': ['single', 'double'],
    },
    'avx2': {
        'flags': {
            'gcc': ['-mavx2', '-mfma'],
            'msvc': ['/arch:AVX2'],
            'ignore_unsupported': {'intel': ['-mfma']},
        },
        'precision': ['single', 'double'],
    },
    'avx512': {
        'condition': host_machine.cpu_family() == 'x86_64',
        'flags': {'gcc': ['-mavx512f'], 'msvc': ['/arch:AVX512']},
        'precision': ['single', 'double'],
    },
    'avx-128-fma': {
        'flags': {'gcc': ['-mfma4']},
        'precision': ['single', 'double'],
    },
    'altivec': {
        'flags': {'gcc': ['-maltivec', '-mabi=altivec']},
        'precision': ['single'],
    },
    'neon': {
        'flags': {
            'gcc': host_machine.cpu_family() == 'aarch64' ? [] : ['-mfpu=neon'],
            'msvc': [],
        },
        'precision': (
            host_machine.cpu_family() == 'aarch64' ?
            [ 'single', 'double', ] : [ 'single', ]
        ),
    },
    'sve': {
        'flags': {'gcc': []},
        'precision': ['single', 'double'],
        'condition': cc.has_header_symbol('arm_sve.h', '__ARM_FEATURE_SVE'),
    },
    'kcvi': {'flags': {'gcc': ['-mmic']}, 'precision': ['single', 'double']},
    'vsx': {'flags': {'gcc': ['-mvsx']}, 'precision': ['single', 'double']},
    'lsx': {'flags': {'gcc': ['-mlsx']}, 'precision': ['single', 'double']},
    'lasx': {'flags': {'gcc': ['-mlasx']}, 'precision': ['single', 'double']},
    'generic-simd128': {'flags': {'gcc': []}, 'precision': ['single', 'double']},
    'generic-simd256': {'flags': {'gcc': []}, 'precision': ['single', 'double']},
}

foreach option, checks : simd_checks
    flags = []
    opt = get_option(option)
    opt = opt.require(
        cc_syntax in checks['flags'],
        error_message: 'not supported with this compiler',
    )

    if 'precision' in checks
        prec_print = ' or '.join(checks['precision'])
        opt = opt.require(
            checks['precision'].contains(precision),
            error_message: f'requires @prec_print@ precision',
        )
    endif

    if 'condition' in checks
        opt = opt.require(
            checks['condition'],
            error_message: 'not supported with this compiler',
        )
    endif

    if opt.allowed()
        foreach flag : checks['flags'][cc_syntax]
            if cc.has_argument(flag)
                flags += flag
            elif not ('ignore_unsupported' in checks['flags']
                and cc_id in checks['flags']['ignore_unsupported']
                and checks['flags']['ignore_unsupported'][cc_id].contains(flag))

                opt = opt.require(
                    false,
                    error_message: f'compiler does not support required flag "@flag@"',
                )
            endif
        endforeach

        if opt.allowed()
            conf.set10('HAVE_' + option.underscorify().to_upper(), true)

            if 'flags_global' in checks and cc_syntax in checks['flags_global']
                add_global_arguments(
                    cc.get_supported_arguments(checks['flags_global'][cc_syntax]),
                    language: 'c',
                )
            endif

        endif
    endif

    summary(option, opt.allowed(), bool_yn: true, section: 'SIMD Optimization')

    set_variable('simd_flags_' + option.underscorify(), flags)
endforeach

subdir('doc')
subdir('api')
subdir('genfft')

# NOTE: Lots of duplicated code there.
# Suggest restructuring if maintainer stuff becomes Meson-exclusive.
subdir('dft')
subdir('rdft')

subdir('reodft')
subdir('kernel')
subdir('simd-support')
subdir('threads')
subdir('libbench2')

configure_file(output: 'config.h', configuration: conf)

pkg = import('pkgconfig')
pcname = 'FFTW'
pcdesc = 'fast Fourier transform library'
pcurl = 'http://fftw.org'
inc = include_directories('api')

fftw_src = fftw_srcset.apply(conf, strict: false)
fftw_lib = library(
    fftw_lib_name,
    [fftw_src.sources()],
    c_shared_args: shlib_args,
    dependencies: fftw_src.dependencies(),
    version: shlib_version,
    install: true,
)
fftw3_dep = declare_dependency(link_with: fftw_lib, include_directories: inc)

pkg.generate(
    fftw_lib,
    filebase: fftw_lib.name(),
    name: pcname,
    description: pcdesc,
    url: pcurl,
)

if fftw_lib_name != 'fftw3'
    pkg.generate(
        fftw_lib,
        filebase: 'fftw3',
        name: pcname,
        description: pcdesc,
        url: pcurl,
    )
endif

if threads_dep.found() and not conf.get('COMBINED_THREADS')
    fftw_threads_src = fftw_threads_srcset.apply(conf)
    fftw_threads_lib = library(
        fftw_lib_name + '_threads',
        fftw_threads_src.sources(),
        c_shared_args: shlib_args,
        link_with: fftw_lib,
        dependencies: fftw_threads_src.dependencies(),
        version: shlib_version,
        install: true,
    )
    fftw3_threads_dep = declare_dependency(
        link_with: fftw_threads_lib,
        include_directories: inc,
    )

    pkg.generate(
        fftw_threads_lib,
        filebase: fftw_threads_lib.name(),
        name: pcname,
        description: pcdesc,
        url: pcurl,
    )
endif

if openmp_dep.found()
    fftw_omp_src = fftw_omp_srcset.apply(conf)
    fftw_omp_lib = library(
        fftw_lib_name + '_omp',
        fftw_omp_src.sources(),
        c_shared_args: shlib_args,
        link_with: fftw_lib,
        dependencies: fftw_omp_src.dependencies(),
        version: shlib_version,
        install: true,
    )

    fftw3_omp_dep = declare_dependency(
        link_with: fftw_omp_lib,
        include_directories: inc,
    )
    pkg.generate(
        fftw_omp_lib,
        filebase: fftw_omp_lib.name(),
        name: pcname,
        description: pcdesc,
        url: pcurl,
    )
endif

subdir('mpi')
subdir('tests')
subdir('tools')

have_cycle_counter = cc.has_header_symbol(
    'kernel/cycle.h',
    'HAVE_TICK_COUNTER',
    prefix: '#include "config.h"',
    include_directories: fftw_root_inc,
)

summary(
    'cycle counter',
    have_cycle_counter ?
    'found' : 'not found - using ESTIMATE mode for all plans (see manual)',
    section: 'Library'
)

# NOTE: Only takes care of the main library.
have_cmake = find_program('cmake', required: false).found()
summary('CMake support files', have_cmake, bool_yn: true, section: 'Build')
if have_cmake
    cmake = import('cmake')

    cmake.write_basic_package_version_file(
        name: 'FFTW3',
        version: shlib_version,
        compatibility: 'SameMajorVersion'
    )
    cmake.configure_package_config_file(
        name: 'FFTW3',
        input: 'support' / 'FFTW3Config.cmake.in',
        configuration: {
            'version': shlib_version,
            'prec_suffix': prec_suffix,
            'lib_name': fftw_lib_name,
            'libdir': get_option('prefix') / get_option('libdir'),
            'includedir': get_option('prefix') / get_option('includedir'),
        }
    )
endif

if is_git and generate_codelets and build_docs
    extra_src = fftw_extra_srcset.all_sources()

    # HACK: This target is called by the dist script so that the SIMD codelet
    # files are generated and collected. This is only needed for Autotools, and
    # can probably be avoided with a small change to make it mirror what we do
    # with Meson: Pass the required SIMD header on the compiler command line.
    # Care should be taken to avoid breaking Makefile dependency tracking, as it
    # is not as robust as Meson’s.
    custom_target('dist_src', input: extra_src, command: 'true', output: '.dist')
    bdir = meson.project_build_root()

    extra_files = []
    foreach file : extra_src
        f = fs.relative_to(file, bdir)
        extra_files += f'"@f@"'
    endforeach

    dist_cfg = configuration_data({'files': ' '.join(extra_files)})
    dist_script = configure_file(
        configuration: dist_cfg,
        input: 'support' / 'meson_dist.sh.in',
        output: 'meson_dist.sh',
    )

    meson.add_dist_script(
        dist_script,
        '@0@.@1@'.format(version_clean[0], version_clean[1]),
        version_clean[2],
        so_current.to_string(),
        so_revision.to_string(),
        so_age.to_string(),
    )
elif is_git
    warning('Git copy but not generating codelets and docs - dist target unavailable')
endif

summary({
    'precision': precision,
    'ABI version': shlib_version
}, section: 'Library')
