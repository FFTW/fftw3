./configure --enable-maintainer-mode --enable-research-mode
make || exit 1
make distcheck || exit 1
./configure --enable-maintainer-mode --disable-research-mode
make || exit 1
make distcheck || exit 1
