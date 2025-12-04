# Meson Build System

A Meson-based build system is included in order to provide a modern alternative
to Autotools for those who need it (such as for better integration with IDEs
and easier inclusion as subproject/dependency of other software).

Note that it is **EXPERIMENTAL**. It should mostly just work, but if you run
into any problems, please report them on our issue tracker and consider using
the Autotools system in the meantime.

It might one day replace Autotools for maintainer tasks, but for the sake of
compatibility, there are currently no plans to remove Autotools: FFTW’s build
tasks are relatively complex, it runs on a wide variety of platforms, and some
(paying) users are not at liberty to drastically alter their build environment.

For that reason, changes to the Meson system should only touch files directly
belonging to it until a decision has been made to promote it to the primary
build system. This means some parts of it will not be as clean and efficient
as they could be.

## Completeness

The Meson build system should have feature parity with Autotools, except for
the following:

* Fortran wrappers (needs replacements for Autoconf-provided macros)
* ARM v7a/v8 performance counters
* Commercialized tarballs

## Requirements

### General

A recent enough version of [Meson](https://mesonbuild.com/) or a compatible
implementation of it (such as [muon](https://sr.ht/~lattis/muon/)) is required.
The exact minimum version is specified at the start of the `meson.build` file.

FFTW itself needs a C compiler. GCC and Clang are well-tested.

ICL and MSVC should also work, but currently only static libraries can be built
successfully with Microsoft’s toolchains. For now, we strongly recommend using
a MinGW-w64-based toolchain to build FFTW for Windows. [MXE](https://mxe.cc/)
can be used to build one if necessary.

Running the test suite currently requires Perl.

### Git only

Additionally, if you have obtained this copy of FFTW from the Git repository,
you will need the [Dune](https://dune.build/) build system for OCaml. Much of
FFTW’s source code is *generated*, and this is required in order to build the
generators. This has only been tested on x86 Linux with glibc.
GNU indent is required as well, since the generated sources in FFTW releases
should be formatted.
Release tarballs already contain the generated sources.

To build the full documentation (already included in release tarballs),
you need:

* Perl
* `makeinfo`
* `fig2dev` (part of transfig)
* `texi2pdf` (part of texinfo).

To generate the Fortran wrappers (included in release tarballs), you need Perl.

## Usage

See the [Meson documentation](https://mesonbuild.com/Manual.html) for
instructions, including usage with IDEs and
[cross compilers](https://mesonbuild.com/Cross-compilation.html).

The short version:

```sh
meson setup builddir
cd builddir
meson compile
meson test --suite small
meson install
```

Some IDEs, like KDevelop, already support Meson. VSCode has an extension
you can install from the marketplace. Once that is set up, you can simply
open the FFTW source directory and everything will be configured automatically.

### Generating Release Tarballs

Assuming you have all of the above and the current Git commit is tagged, you
can simply run `meson dist` in a configured build directory. This will do the
following by default:

* Collect the sources in the current Git `HEAD` commit
  NOTE: NOT whatever is currently checked out, and thus NOT including untracked
  or modified files! Meson will warn about this and require a command line flag
  to proceed if it detects uncommitted changes.
* Cement the tarball’s version (so it won’t depend on Git)
* Generate `ChangeLog` from Git
* Generate all codelets and copy them to the tarball tree
* Patch `configure.ac` to match Meson’s version info
* Configure the resulting sources and run the full test suite
* Upon success, generate a compressed tarball and accompanying checksum file

If the commit is not tagged, the build system still packages a tarball, but it
will print a warning and refuse to include the Autotools build system.

Most of the above happens in a shell script generated from
`support/meson_dist.sh.in`.
