# build normal and fma distributions

NJOBS=2

cvs update -d

# hackery to build ChangeLog
echo >ChangeLog
rcs2log -l 72 >ChangeLog
emacs -batch -q -no-site-file --eval \
   '(progn (find-file "ChangeLog") (fill-region 1 1000000) (save-buffer))'

sh bootstrap.sh

make maintainer-clean
./configure --enable-maintainer-mode --disable-fma
make -j $NJOBS
make -j $NJOBS dist

