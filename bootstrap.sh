# script to initialize automake/autoconf etc
echo "Please ignore warnings and errors"
touch ChangeLog
touch genfft/.depend
touch genfft-k7/.depend
autoheader
aclocal
automake --add-missing
autoconf
libtoolize --automake
autoheader
aclocal
automake --add-missing
autoconf
libtoolize --automake

rm -f config.cache

# --enable-maintainer-mode enables build of genfft and automatic
# rebuild of codelets whenever genfft changes
(
    ./configure --disable-shared --enable-maintainer-mode #--enable-debug
    (cd genfft; make depend)
    (cd genfft-k7; make depend)
)
