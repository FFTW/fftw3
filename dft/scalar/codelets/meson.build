genfft_params = {
    ###########################################################################
    # n1_<n> is a hard-coded FFT of size <n> (base cases of FFT recursion)
    'n1': {
        'generator': gen_notw,
        'include': 'n.h',
        'extra_args': [],
        'ns': [
            2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 32, 64, 20, 25,
            # 30, 40, 50,
        ],
    },
    ###########################################################################
    # t1_<r> is a "twiddle" FFT of size <r>, implementing a radix-r DIT step
    't1': {
        'generator': gen_twiddle,
        'include': 't.h',
        'extra_args': [],
        'ns': [
            2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 32, 64, 20, 25,
            # 30, 40, 50
        ],
    },
    # t2_<r> is also a twiddle FFT, but instead of using a complete lookup table
    # of trig. functions, it partially generates the trig. values on the fly
    # (this is faster for large sizes).
    't2': {
        'generator': gen_twiddle,
        'include': 't.h',
        'extra_args': [
            '-twiddle-log3',
            '-precompute-twiddles'
        ],
        'ns': [
            4, 8, 16, 32, 64,
            5, 10, 20, 25
        ],
    },
    ###########################################################################
    # The F (DIF) codelets are used for a kind of in-place transform algorithm,
    # but the planner seems to never (or hardly ever) use them on the machines
    # we have access to, preferring the Q codelets and the use of buffers
    # for sub-transforms.  So, we comment them out, at least for now.

    # f1_<r> is a "twiddle" FFT of size <r>, implementing a radix-r DIF step
    'f1': {
        'generator': gen_twiddle,
        'include': 'f.h',
        'extra_args': [
            '-dif'
        ],
        'ns': [
            # 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 32, 64,
        ],
    },
    # like f1, but partially generates its trig. table on the fly
    'f2': {
        'generator': gen_twiddle,
        'include': 'f.h',
        'extra_args': [
            '-dif',
            '-twiddle-log3',
            '-precompute-twiddles'
        ],
        'ns': [
            # 4, 8, 16, 32, 64,
        ],
    },
    ###########################################################################
    # q1_<r> is <r> twiddle FFTs of size <r> (DIF step), where the output is
    # transposed.  This is used for in-place transposes in sizes that are
    # divisible by <r>^2.  These codelets have size ~ <r>^2, so you should
    # probably not use <r> bigger than 8 or so.
    'q1': {
        'generator': gen_twidsq,
        'include': 'q.h',
        'extra_args': [
            '-dif',
            '-reload-twiddle'
        ],
        'ns': [
            2, 4, 8,
            3, 5, 6
        ],
    },
    'q2': {
        'generator': gen_twidsq,
        'include': 'q.h',
        'extra_args': [
            '-dif',
            '-twiddle-log3',
            '-precompute-twiddles'
        ],
        'ns': [],
    },
}

flags_common = [
    '-compact',
    '-variables', '4',
    '-pipeline-latency', '4'
]

incdir = fs.relative_to(
    meson.current_source_dir() / '..',
    meson.project_source_root(),
)

codelets = []
foreach type, params : genfft_params
    foreach n : params['ns']
        codelet = f'@type@_@n@'
        codelets += codelet
        outname = codelet + '.c'

        if generate_codelets
            tgt = custom_target(
                outname,
                output: outname,
                capture: true,
                command: [
                    gen_codelet,
                    codelet_prelude,
                    params['generator'],
                    flags_common,
                    params['extra_args'],
                    '-include', incdir / params['include'],
                    '-n', f'@n@',
                    '-name', codelet,
                ],
            )
            fftw_srcset.add(tgt)
            fftw_extra_srcset.add(tgt)
        else
            fftw_srcset.add(files(outname))
        endif
    endforeach
endforeach

if generate_codelets
    codlist = configuration_data(
        {'solvtab_name': 'X(solvtab_dft_standard)', 'extra_includes': ''},
    )

    decls = []
    foreach codelet : codelets
        decls += f'extern void X(codelet_@codelet@)(planner *);'
    endforeach
    codlist.set('decls', '\n'.join(decls))

    solvtab_entries = []
    foreach codelet : codelets
        solvtab_entries += f'   SOLVTAB(X(codelet_@codelet@)),'
    endforeach
    codlist.set('solvtab_entries', '\n'.join(solvtab_entries))

    cfgf = configure_file(
        configuration: codlist,
        input: support_dir / 'codlist.c.in',
        output: 'codlist.c',
    )
    fftw_srcset.add(cfgf)
    fftw_extra_srcset.add(cfgf)
else
    fftw_srcset.add(files('codlist.c'))
endif
