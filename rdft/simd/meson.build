# This file contains a standard list of RDFT SIMD codelets.  It is
# used by common/meson.build to generate the C files with the actual
# codelets in them.  It is used by {sse,sse2,...}/meson.build to
# generate and compile stub files that include common/*.c

# You can customize FFTW for special needs, e.g. to handle certain
# sizes more efficiently, by adding new codelets to the lists of those
# included by default.  If you change the list of codelets, any new
# ones you added will be automatically generated when you run the
# bootstrap script (see "Generating your own code" in the FFTW
# manual).

genfft_params_simd = {
    'hc2cfdftv': {
        'generator': gen_hc2cdft_c,
        'include': 'hc2cfv.h',
        'extra_args': [
            '-dit'
        ],
        'ns': [
            2, 4, 6, 8, 10, 12, 16, 32,
            20
        ],
    },
    'hc2cbdftv': {
        'generator': gen_hc2cdft_c,
        'include': 'hc2cbv.h',
        'extra_args': [
            '-dif',
            '-sign', '1'
        ],
        'ns': [
            2, 4, 6, 8, 10, 12, 16, 32,
            20
        ],
    },
}

subdir('common')

subdirs = {
    'HAVE_ALTIVEC': {
        'dirs': {'altivec' : 'simd-support/simd-altivec.h'},
        'flags': simd_flags_altivec,
    },
    'HAVE_SSE2': {
        'dirs': {'sse2' : 'simd-support/simd-sse2.h'},
        'flags': simd_flags_sse2,
    },
    'HAVE_AVX': {
        'dirs': {'avx' : 'simd-support/simd-avx.h'},
        'flags': simd_flags_avx,
    },
    'HAVE_AVX_128_FMA': {
        'dirs': {'avx-128-fma' : 'simd-support/simd-avx-128-fma.h'},
        'flags': simd_flags_avx_128_fma,
    },
    'HAVE_AVX2': {
        'dirs': {
            'avx2' : 'simd-support/simd-avx2.h',
            'avx2-128' : 'simd-support/simd-avx2-128.h',
        },
        'flags': simd_flags_avx2,
    },
    'HAVE_AVX512': {
        'dirs': {'avx512' : 'simd-support/simd-avx512.h'},
        'flags': simd_flags_avx512,
    },
    'HAVE_KCVI': {
        'dirs': {'kcvi' : 'simd-support/simd-kcvi.h'},
        'flags': simd_flags_kcvi,
    },
    'HAVE_GENERIC_SIMD128': {
        'dirs': {'generic-simd128' : 'simd-support/simd-generic128.h'},
        'flags': simd_flags_generic_simd128,
    },
    'HAVE_GENERIC_SIMD256': {
        'dirs': {'generic-simd256' : 'simd-support/simd-generic256.h'},
        'flags': simd_flags_generic_simd256,
    },
    'HAVE_LASX': {
        'dirs': {'lasx' : 'simd-support/simd-lasx.h'},
        'flags': simd_flags_lasx,
    },
    'HAVE_LSX': {
        'dirs': {'lsx' : 'simd-support/simd-lsx.h'},
        'flags': simd_flags_lsx,
    },
    'HAVE_NEON': {
        'dirs': {'neon' : 'simd-support/simd-neon.h'},
        'flags': simd_flags_neon,
    },
    'HAVE_SVE': {
        'dirs': {
            'sve128' : 'simd-support/simd-maskedsve128.h',
            'sve256' : 'simd-support/simd-maskedsve256.h',
            'sve512' : 'simd-support/simd-maskedsve512.h',
            'sve1024' : 'simd-support/simd-maskedsve1024.h',
            'sve2048' : 'simd-support/simd-maskedsve2048.h',
        },
        'flags': simd_flags_sve,
    },
    'HAVE_VSX': {
        'dirs': {'vsx' : 'simd-support/simd-vsx.h'},
        'flags': simd_flags_vsx,
    },
}

foreach cond, simd : subdirs
    simd_flags = simd['flags']
    foreach dir, header : simd['dirs']
        simd_header = header
        simd_name = dir
        subdir(dir)

        if conf.has(cond)
            lib = static_library(
                simd_name,
                build_by_default: false,
                c_args: [simd_flags, f'@cc_def@SIMD_HEADER="@simd_header@"'],
                include_directories: fftw_root_inc,
                sources: simd_common_codelets,
            )

            fftw_srcset.add(declare_dependency(link_with: lib))
        endif
    endforeach
endforeach
