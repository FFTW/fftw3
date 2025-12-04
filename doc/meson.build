docs_opt = get_option('docs')
makeinfo = find_program('makeinfo', required: docs_opt)
perl = find_program('perl', required: docs_opt)
fig2dev = find_program('fig2dev', required: docs_opt)
texi2pdf = find_program('texi2pdf', required: docs_opt)

build_docs =  makeinfo.found() and perl.found() and fig2dev.found() and texi2pdf.found()

summary('Rebuild documentation', build_docs, bool_yn: true, section: 'Build')

if not build_docs
    subdir_done()
endif

version_texi = custom_target(
    input: 'fftw3.texi',
    output: 'version.texi',
    capture: true,
    command: [
        perl,
        files('mkversion.pl'),
        '@INPUT@',
        meson.project_version(),
    ],
)

fftw_extra_srcset.add(version_texi)

rfftwnd_eps = custom_target(
    input: 'rfftwnd.fig',
    output: '@BASENAME@.eps',
    command: [
        fig2dev,
        '-L', 'eps',
        '-m', '.7',
        '@INPUT@',
        '@OUTPUT@'
    ],
)

rfftwnd_for_html_png = custom_target(
    input: 'rfftwnd.fig',
    output: '@BASENAME@-for-html.png',
    command: [
        fig2dev,
        '-L', 'png',
        '-m', '1',
        '-S', '4',
        '@INPUT@',
        '@OUTPUT@',
    ],
)

rfftwnd_pdf = custom_target(
    input: 'rfftwnd.fig',
    output: '@BASENAME@.pdf',
    command: [
        fig2dev,
        '-L', 'pdf',
        '-m', '.7',
        '@INPUT@',
        '@OUTPUT@'
    ],
)

fftw_extra_srcset.add(rfftwnd_eps)
fftw_extra_srcset.add(rfftwnd_for_html_png)
fftw_extra_srcset.add(rfftwnd_pdf)

texifiles = files(
    'acknowledgements.texi',
    'cindex.texi',
    'fftw3.texi',
    'findex.texi',
    'install.texi',
    'intro.texi',
    'legacy-fortran.texi',
    'license.texi',
    'modern-fortran.texi',
    'mpi.texi',
    'other.texi',
    'reference.texi',
    'threads.texi',
    'tutorial.texi',
    'upgrading.texi',
)
eqn_images = files(
    'equation-dft.png',
    'equation-dht.png',
    'equation-idft.png',
    'equation-redft00.png',
    'equation-redft01.png',
    'equation-redft10.png',
    'equation-redft11.png',
    'equation-rodft00.png',
    'equation-rodft01.png',
    'equation-rodft10.png',
    'equation-rodft11.png',
)

info = custom_target(
    input: [texifiles, version_texi],
    output: 'fftw3.info',
    command: [
        makeinfo,
        '-I', meson.current_source_dir(),
        '-I', meson.current_build_dir(),
        '--number-sections',
        '--no-split',
        '-o', '@OUTPUT@',
        files('fftw3.texi'),
    ],
)
fftw_extra_srcset.add(info)

html = custom_target(
    input: [texifiles, version_texi, rfftwnd_for_html_png, eqn_images],
    output: 'html',
    command: [
        makeinfo,
        '-I', meson.current_source_dir(),
        '-I', meson.current_build_dir(),
        '--html',
        '--number-sections',
        '-o', '@OUTPUT@',
        files('fftw3.texi'),
    ],
)
fftw_extra_srcset.add(html)

pdf = custom_target(
    input: [texifiles, version_texi, rfftwnd_pdf, eqn_images],
    output: 'fftw3.pdf',
    command: [
        texi2pdf,
        '-q',
        '-I', meson.current_source_dir(),
        '-I', meson.current_build_dir(),
        '--pdf',
        '-o', '@OUTPUT@',
        files('fftw3.texi'),
    ],
)
fftw_extra_srcset.add(pdf)
