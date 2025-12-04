flags_common = [
    '-compact',
    '-variables', '4',
    '-pipeline-latency', '8',
    '-simd',
]

incdir = fs.relative_to(
    meson.current_source_dir() / '..',
    meson.project_source_root(),
)

simd_common_codelets = []
codelets = []
foreach type, params : genfft_params_simd
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
            simd_common_codelets += tgt
        else
            simd_common_codelets += files(outname)
        endif
    endforeach
endforeach

if generate_codelets
    codlist = configuration_data(
        {
            'solvtab_name': 'XSIMD(solvtab_dft)',
            'extra_includes': '#include SIMD_HEADER',
        },
    )

    decls = []
    foreach codelet : codelets
        decls += f'extern void XSIMD(codelet_@codelet@)(planner *);'
    endforeach
    codlist.set('decls', '\n'.join(decls))

    solvtab_entries = []
    foreach codelet : codelets
        solvtab_entries += f'   SOLVTAB(XSIMD(codelet_@codelet@)),'
    endforeach
    codlist.set('solvtab_entries', '\n'.join(solvtab_entries))

    simd_common_codelets += \
    configure_file(
        configuration: codlist,
        input: support_dir / 'codlist.c.in',
        output: 'codlist.c',
    )
    simd_common_codelets += \
    configure_file(input: 'genus.c', output: 'genus.c', copy: true)
else
    simd_common_codelets += files('codlist.c', 'genus.c')
endif

fftw_extra_srcset.add(simd_common_codelets)
