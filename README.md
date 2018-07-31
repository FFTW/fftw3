*DO NOT CHECK OUT THESE FILES FROM GITHUB UNLESS YOU KNOW WHAT YOU ARE
DOING.*  (See below.)

This is the git repository for the FFTW library for computing Fourier
transforms (version 3.x), maintained by the FFTW authors.

Unlike most other programs, most of the FFTW source code (in C) is
generated automatically.  This repository contains the *generator* and
it does not contain the *generated code*.  *YOU WILL BE UNABLE TO
COMPILE CODE FROM THIS REPOSITORY* unless you have special tools and
know what you are doing.   In particular, do not expect things to
work by simply executing `configure; make` or `cmake`.

Most users should ignore this repository, and should instead download
official tarballs from http://fftw.org/, which contain the generated
code, do not require any special tools or knowledge, and can be
compiled on any system with a C compiler.

Advanced users and FFTW maintainers may obtain code from github and
run the generation process themselves.  See README for details.

