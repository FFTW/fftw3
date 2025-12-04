if generate_codelets
    foreach ccodelet: simd_common_codelets
        tgt = custom_target(input: ccodelet, output: '@PLAINNAME@',
                            capture: true,
                            command: [gen_codelet_simd, simd_header, '@PLAINNAME@'])

        # We really only need these for dist tarballs.
        fftw_extra_srcset.add(tgt)
    endforeach
endif
