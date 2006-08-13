NJOBS=2

darcs pull fftw@fftw.org:darcs/fftw3

# hackery to build ChangeLog
darcs changes >ChangeLog
emacs -batch -q -no-site-file --eval \
   '(progn (find-file "ChangeLog") (fill-region 1 1000000) (save-buffer))'

sh bootstrap.sh

make maintainer-clean
./configure --enable-maintainer-mode --enable-single --enable-sse --enable-k7 --enable-threads
make -j $NJOBS
make -j $NJOBS dist

