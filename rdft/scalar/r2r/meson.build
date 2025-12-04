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
    # The following lines specify the REDFT/RODFT/DHT sizes for which to generate
    # specialized codelets.  Currently, only REDFT01/10 of size 8 (used in JPEG).

    # e<a><b>_<n> is a hard-coded REDFT<a><b> FFT (DCT) of size <n>
    'e00': {
        'generator': gen_r2r,
        'include': 'r2r.h',
        'extra_args': [
            '-redft00'
        ],
        'ns': [
            # 2, 3, 4, 5, 6, 7, 8,
        ],
    },
    'e01': {
        'generator': gen_r2r,
        'include': 'r2r.h',
        'extra_args': [
            '-redft01'
        ],
        'ns': [
            8,
            # 2, 3, 4, 5, 6, 7,
        ],
    },
    'e10': {
        'generator': gen_r2r,
        'include': 'r2r.h',
        'extra_args': [
            '-redft10'
        ],
        'ns': [
            8,
            # 2, 3, 4, 5, 6, 7,
        ],
    },
    'e11': {
        'generator': gen_r2r,
        'include': 'r2r.h',
        'extra_args': [
            '-redft11'
        ],
        'ns': [
            # 2, 3, 4, 5, 6, 7, 8,
        ],
    },
    # o<a><b>_<n> is a hard-coded RODFT<a><b> FFT (DST) of size <n>
    'o00': {
        'generator': gen_r2r,
        'include': 'r2r.h',
        'extra_args': [
            '-rodft00'
        ],
        'ns': [
            # 2, 3, 4, 5, 6, 7, 8,
        ],
    },
    'o01': {
        'generator': gen_r2r,
        'include': 'r2r.h',
        'extra_args': [
            '-rodft01'
        ],
        'ns': [
            # 2, 3, 4, 5, 6, 7, 8,
        ],
    },
    'o10': {
        'generator': gen_r2r,
        'include': 'r2r.h',
        'extra_args': [
            '-rodft10'
        ],
        'ns': [
            # 2, 3, 4, 5, 6, 7, 8,
        ],
    },
    'o11': {
        'generator': gen_r2r,
        'include': 'r2r.h',
        'extra_args': [
            '-rodft11'
        ],
        'ns': [
            # 2, 3, 4, 5, 6, 7, 8,
        ],
    },
    # dht_<n> is a hard-coded DHT of size <n>
    'dht': {
        'generator': gen_r2r,
        'include': 'r2r.h',
        'extra_args': [
            '-dht',
            '-sign', '1'
        ],
        'ns': [
            # 2, 3, 4, 5, 6, 7, 8,
        ],
    },
}

flags_common = [
    '-compact',
    '-variables', '4',
    '-pipeline-latency', '4'
]
solvtab_name = 'X(solvtab_rdft_r2r)'

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
