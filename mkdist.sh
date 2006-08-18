NJOBS=2

darcs pull fftw@fftw.org:darcs/fftw3

# hackery to build ChangeLog
darcs changes --summary > ChangeLog

sh bootstrap.sh

make maintainer-clean
./configure --enable-maintainer-mode --enable-single --enable-sse --enable-threads
make -j $NJOBS
make -j $NJOBS dist

