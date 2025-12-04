# This file specifies a set of codelets, efficient transforms
# of small sizes, that are used as building blocks (kernels) by FFTW
# to build up large transforms, as well as the options for generating
# and compiling them.

# You can customize FFTW for special needs, e.g. to handle certain
# sizes more efficiently, by adding new codelets to the lists of those
# included by default.  If you change the list of codelets, any new
# ones you added will be automatically generated when you run the
# bootstrap script (see "Generating your own code" in the FFTW
# manual).

genfft_params = {
    ###########################################################################
    # r2cf_<n> is a hard-coded real-to-complex FFT of size <n> (base cases
    # of real-input FFT recursion)
    'r2cf': {
        'generator': gen_r2cf,
        'include': 'r2cf.h',
        'extra_args': [],
        'ns': [
            2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 32, 64, 128,
            20, 25,
            # 30, 40, 50,
        ],
    },
    ###########################################################################
    # hf_<r> is a "twiddle" FFT of size <r>, implementing a radix-r DIT
    # step for a real-input FFT.  Every hf codelet must have a
    # corresponding r2cfII codelet (see below)!
    'hf': {
        'generator': gen_hc2hc,
        'include': 'hf.h',
        'extra_args': [
            '-dit'
        ],
        'ns': [
            2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 32, 64,
            20, 25,
            # 30, 40, 50,
        ],
    },
    # like hf, but generates part of its trig table on the fly (good for large n)
    'hf2': {
        'generator': gen_hc2hc,
        'include': 'hf.h',
        'extra_args': [
            '-dit',
            '-twiddle-log3',
            '-precompute-twiddles'
        ],
        'ns': [
            4, 8, 16, 32,
            5, 20, 25
        ],
    },
    # an r2cf transform where the input is shifted by half a sample (output
    # is multiplied by a phase).  This is needed as part of the DIT recursion;
    # every hf_<r> or hf2_<r> codelet should have a corresponding r2cfII_<r>
    'r2cfII': {
        'generator': gen_r2cf,
        'include': 'r2cfII.h',
        'extra_args': [
            '-dft-II'
        ],
        'ns': [
            2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 32, 64,
            20, 25,
            # 30, 40, 50,
        ],
    },
    ###########################################################################
    # hc2cf_<r> is a "twiddle" FFT of size <r>, implementing a radix-r DIT
    # step for a real-input FFT with rdft2-style output.  <r> must be even.
    'hc2cf': {
        'generator': gen_hc2c,
        'include': 'hc2cf.h',
        'extra_args': [
            '-dit'
        ],
        'ns': [
            2, 4, 6, 8, 10, 12, 16, 32,
            20,
            # 30,
        ],
    },
    'hc2cfdft': {
        'generator': gen_hc2cdft,
        'include': 'hc2cf.h',
        'extra_args': [
            '-dit'
        ],
        'ns': [
            2, 4, 6, 8, 10, 12, 16, 32,
            20,
            # 30,
        ],
    },
    # like hc2cf, but generates part of its trig table on the fly (good
    # for large n)
    'hc2cf2': {
        'generator': gen_hc2c,
        'include': 'hc2cf.h',
        'extra_args': [
            '-dit',
            '-twiddle-log3',
            '-precompute-twiddles'
        ],
        'ns': [
            4, 8, 16, 32,
            20,
            # 30,
        ],
    },
    'hc2cfdft2': {
        'generator': gen_hc2cdft,
        'include': 'hc2cf.h',
        'extra_args': [
            '-dit',
            '-twiddle-log3',
            '-precompute-twiddles'
        ],
        'ns': [
            4, 8, 16, 32,
            20,
            # 30,
        ],
    },
}

flags_common = [
    '-compact',
    '-variables', '4',
    '-pipeline-latency', '4'
]
solvtab_name = 'X(solvtab_rdft_r2cf)'

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
        {'solvtab_name': solvtab_name, 'extra_includes': ''},
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
