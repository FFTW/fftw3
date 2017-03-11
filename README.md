*DO NOT CHECK OUT THESE FILES FROM GITHUB UNLESS YOU KNOW WHAT YOU ARE
DOING.*  (See below.)

This is the git repository for the FFTW library for computing Fourier
transforms (version 3.x), maintained by the FFTW authors.

Unlike most other programs, most of the FFTW source code (in C) is
generated automatically.  This repository contains the *generator* and
it does not contain the *generated code*.  You cannot compile code
from this repository unless you have special tools and know what you
are doing.

Most users should ignore this repository, and should instead download
official tarballs from http://fftw.org/, which contain the generated
code, do not require any special tools or knowledge, and can be
compiled on any system with a C compiler.

Advanced users and FFTW maintainers can obtain code from github and
run the generation process themselves.  This is a long process that
requires special tools.  See README for details.  (Summary: in
addition to the usual Unix developer software, you need [GNU
autotools](https://en.wikipedia.org/wiki/GNU_Build_System) and
[OCaml](http://www.ocaml.org/).  Then you can run `sh mkdist.sh`
to compile FFTW and generate `.tar.gz` files similar to the official
releases.)

