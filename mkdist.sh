# This is a script used by the FFTW authors to generate FFTW distributions.
# You should never need to use it.

NJOBS=4

tag=`git tag --contains HEAD`
if [ -z "$tag" ]; then
    echo "Current git HEAD is not tagged---refusing to build distribution"
    exit 1
fi

# hackery to build ChangeLog
git log --pretty=medium --date-order > ChangeLog

sh bootstrap.sh

make maintainer-clean
./configure --enable-maintainer-mode --enable-single --enable-sse --enable-threads
make -j $NJOBS
make dist

