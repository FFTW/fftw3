#!/bin/bash -e
# This is a script used by the FFTW authors to generate FFTW distributions.
# You should never need to use it.

NJOBS=${NJOBS:-4}

tag=$(git tag --points-at HEAD)
if [ -z "$tag" ]; then
    echo "Current git HEAD is not tagged---refusing to build distribution"
    exit 1
fi

# hackery to build ChangeLog
git log --pretty=medium --date-order > ChangeLog

python3 support/code_generation.py -j "$NJOBS"
sh autogen.sh
./configure --enable-maintainer-mode "$@"
make -j "$NJOBS" dist
