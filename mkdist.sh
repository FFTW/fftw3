./configure --enable-maintainer-mode --disable-classic-mode
make || exit 1
make distcheck || exit 1
./configure --enable-maintainer-mode --enable-classic-mode
make || exit 1
make distcheck || exit 1
