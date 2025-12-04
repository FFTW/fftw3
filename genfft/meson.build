gen_opt = get_option('generate_codelets')
gen_opt = gen_opt.enable_auto_if(is_git)
gen_opt = gen_opt.disable_auto_if(not is_git)

dune = find_program('dune', required: gen_opt)
indent = find_program('indent', required: gen_opt)
generate_codelets = dune.found() and indent.found()

summary(
    'Generate codelets',
    generate_codelets,
    bool_yn: true,
    section: 'Build'
)

if not dune.found()
    dune = disabler()
endif

genfft_progs = [
    'gen_notw',
    'gen_notw_c',
    'gen_twiddle',
    'gen_twiddle_c',
    'gen_twidsq',
    'gen_twidsq_c',
    'gen_r2r',
    'gen_r2cf',
    'gen_r2cb',
    'gen_hc2c',
    'gen_hc2cdft',
    'gen_hc2cdft_c',
    'gen_hc2hc',
    'gen_mdct',
]

genfft_progs_exe = []
foreach prog : genfft_progs
    genfft_progs_exe += prog + '.exe'
endforeach

dune_args = [
    '--root=' + meson.current_source_dir(),
    '--build-dir=' + meson.current_build_dir() / '..',
    '--no-print-directory',
]

dune_lock = custom_target(
    output: 'dune.lock',
    command: [dune, 'pkg', 'lock', dune_args],
    build_by_default: false,
)

run_target(
    'dune-pkg',
    command: [dune, 'build', dune_args, '@pkg-install'],
    depends: dune_lock,
)

# Need to build all at once instead of dependency-driven to avoid Dune
# deadlocking itself
genfft_exe = custom_target(
    'genfft',
    output: genfft_progs_exe,
    command: [dune, 'build', dune_args],
    # Dune will manage its dependencies
    build_always_stale: true,
)

foreach i : range(genfft_progs.length())
    set_variable(genfft_progs[i], genfft_exe[i])
endforeach

if generate_codelets
    gen_codelet = [
        support_dir / 'gen_codelet.sh',
        meson.project_source_root() / 'COPYRIGHT',
        support_dir / 'twovers.sh',
        indent.full_path(),
    ]
    gen_codelet_simd = support_dir / 'gen_codelet_simd.sh'
endif
