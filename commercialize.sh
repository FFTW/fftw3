#! /bin/sh

# This is a script used by the FFTW copyright holders to
# create commercial versions with a non-GPL license.
# You should never need to use it.
tarball=$1
newtarball="commercial-"`basename $tarball`

echo "Commercializing $tarball to produce $newtarball"

d=`basename $tarball .tar.gz`
rm -rf $d
tar xpzf $tarball

find $d -type f -print | while read name; do
    sed -e '/^ [*] This program is free software; you can redistribute it and\/or modify$/,/ [*] Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA/c\
 * See the file COPYING for license information.' $name > ${name}.tmp
    chmod --reference=$name ${name}.tmp
    touch --reference=$name ${name}.tmp
    mv ${name}.tmp $name
done

for name in $d/tools/fftw-wisdom.c; do
    sed -e '/This program is free software; you can redistribute it and\/or modify/,/["]Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA..["]/c\
"See the file COPYING for license information.\\n"' $name > ${name}.tmp
    chmod --reference=$name ${name}.tmp
    touch --reference=$name ${name}.tmp
    mv ${name}.tmp $name
done

for name in $d/configure.ac; do
    cat $name | sed -e 's+AC_INIT(fftw,+AC_INIT(commercial-fftw,+' > ${name}.tmp
    chmod --reference=$name ${name}.tmp
    touch --reference=$name ${name}.tmp
    mv ${name}.tmp $name
done

(
 cd $d; 
 rm -rf autom4te.cache
 autoreconf --verbose --install --symlink --force
 autoreconf --verbose --install --symlink --force
 autoreconf --verbose --install --symlink --force
 rm -f config.cache

 rm -f COPYING
 cat >COPYING <<EOF
This package is licensed commercially by the MIT Technology Licensing
Office (TLO); you should have a license agreement describing the terms
you have negotiated.

Please note that this package is provided WITHOUT ANY WARRANTY.
See your license agreement for complete details.

Contact Mr. Thomas O'Keefe (tokeefe@MIT.EDU) for more information
regarding licensing.
EOF


 for name in simd/*mips*.[ch]; do
 cat >$name <<EOF
/* This file is (C) Codesourcery and removed from the commercial
   version of FFTW */
EOF
 done

 ./configure --enable-sse2
 make dist
 mv $newtarball ..
)

rm -rf $d
