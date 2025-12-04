# This file contains a standard list of DFT SIMD codelets.  It is
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
    ###########################################################################
    # n1fv_<n> is a hard-coded FFTW_FORWARD FFT of size <n>, using SIMD
    'n1fv': {
        'generator': gen_notw_c,
        'include': 'n1f.h',
        'extra_args': [],
        'ns': [
            2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 32, 64, 128,
            20, 25,
        ],
    },
    # as above, with restricted input vector stride
    'n2fv': {
        'generator': gen_notw_c,
        'include': 'n2f.h',
        'extra_args': [
            '-with-ostride', '2',
            '-store-multiple', '2'
        ],
        'ns': [2, 4, 6, 8, 10, 12, 14, 16, 32, 64, 20],
    },
    # as above, but FFTW_BACKWARD
    'n1bv': {
        'generator': gen_notw_c,
        'include': 'n1b.h',
        'extra_args': [
            '-sign', '1'
        ],
        'ns': [
            2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 32, 64, 128,
            20, 25,
        ],
    },
    'n2bv': {
        'generator': gen_notw_c,
        'include': 'n2b.h',
        'extra_args': [
            '-sign', '1',
            '-with-ostride', '2',
            '-store-multiple', '2',
        ],
        'ns': [
            2, 4, 6, 8, 10, 12, 14, 16, 32, 64, 20
        ],
    },
    # split-complex codelets
    'n2sv': {
        'generator': gen_notw,
        'include': 'n2s.h',
        'extra_args': [
            '-with-ostride', '1',
            '-store-multiple', '4'
        ],
        'ns': [
            4, 8, 16, 32, 64
        ],
    },
    ###########################################################################
    # t1fv_<r> is a "twiddle" FFT of size <r>, implementing a radix-r DIT step
    # for an FFTW_FORWARD transform, using SIMD
    't1fv': {
        'generator': gen_twiddle_c,
        'include': 't1f.h',
        'extra_args': [],
        'ns': [
            2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 32, 64,
            20, 25
        ],
    },
    # same as t1fv_*, but with different twiddle storage scheme
    't2fv': {
        'generator': gen_twiddle_c,
        'include': 't2f.h',
        'extra_args': [],
        'ns': [
            2, 4, 8, 16, 32, 64, 5, 10,
            20, 25
        ],
    },
    't3fv': {
        'generator': gen_twiddle_c,
        'include': 't3f.h',
        'extra_args': [
            '-twiddle-log3',
            '-precompute-twiddles',
            '-no-generate-bytw',
        ],
        'ns': [
            4, 8, 16, 32, 5, 10, 20, 25
        ],
    },
    't1fuv': {
        'generator': gen_twiddle_c,
        'include': 't1fu.h',
        'extra_args': [],
        'ns': [
            2, 3, 4, 5, 6, 7, 8, 9, 10
        ],
    },
    # as above, but FFTW_BACKWARD
    't1bv': {
        'generator': gen_twiddle_c,
        'include': 't1b.h',
        'extra_args': [
            '-sign', '1'
        ],
        'ns': [
            2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 32, 64,
            20, 25
        ],
    },
    # same as t1bv_*, but with different twiddle storage scheme
    't2bv': {
        'generator': gen_twiddle_c,
        'include': 't2b.h',
        'extra_args': [
            '-sign', '1'
        ],
        'ns': [
            2, 4, 8, 16, 32, 64,
            5, 10, 20, 25
        ],
    },
    't3bv': {
        'generator': gen_twiddle_c,
        'include': 't3b.h',
        'extra_args': [
            '-twiddle-log3',
            '-precompute-twiddles',
            '-no-generate-bytw',
            '-sign', '1',
        ],
        'ns': [
            4, 8, 16, 32,
            5, 10, 20, 25
        ],
    },
    't1buv': {
        'generator': gen_twiddle_c,
        'include': 't1bu.h',
        'extra_args': [
            '-sign', '1'
        ],
        'ns': [
            2, 3, 4, 5, 6, 7, 8, 9, 10
        ],
    },
    # split-complex codelets
    't1sv': {
        'generator': gen_twiddle,
        'include': 'ts.h',
        'extra_args': [],
        'ns': [
            2, 4, 8, 16, 32
        ],
    },
    't2sv': {
        'generator': gen_twiddle,
        'include': 'ts.h',
        'extra_args': [
            '-twiddle-log3',
            '-precompute-twiddles'
        ],
        'ns': [
            4, 8, 16, 32
        ],
    },
    ###########################################################################
    # q1fv_<r> is <r> twiddle FFTW_FORWARD FFTs of size <r> (DIF step),
    # where the output is transposed, using SIMD.  This is used for
    # in-place transposes in sizes that are divisible by <r>^2.  These
    # codelets have size ~ <r>^2, so you should probably not use <r>
    # bigger than 8 or so.
    'q1fv': {
        'generator': gen_twidsq_c,
        'include': 'q1f.h',
        'extra_args': [
            '-dif'
        ],
        'ns': [
            2, 4, 5, 8
        ],
    },
    'q1bv': {
        'generator': gen_twidsq_c,
        'include': 'q1b.h',
        'extra_args': [
            '-dif',
            '-sign', '1'
        ],
        'ns': [
            2, 4, 5, 8
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
