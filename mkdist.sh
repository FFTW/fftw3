# build normal and fma distributions

cvs update -d
sh bootstrap.sh

make maintainer-clean
./configure --enable-maintainer-mode --enable-fma
make -j 8
make -j 8 dist

make maintainer-clean
./configure --enable-maintainer-mode --disable-fma
make -j 8
make -j 8 dist

