#! /bin/sh
# script to initialize automake/autoconf etc

touch ChangeLog
touch genfft/.depend
touch genfft-k7/.depend

echo "PLEASE IGNORE WARNINGS AND ERRORS"

# paranoia: sometimes autoconf doesn't get things right the first time
autoreconf --verbose --install --symlink --force
autoreconf --verbose --install --symlink --force
autoreconf --verbose --install --symlink --force

rm -f config.cache

# --enable-maintainer-mode enables build of genfft and automatic
# rebuild of codelets whenever genfft changes
(
    ./configure --disable-shared --enable-maintainer-mode $*
    (cd genfft; make depend)
    (cd genfft-k7; make depend)
)
