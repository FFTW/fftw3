#! /bin/sh

tarball=$1
newtarball="commercial-"`basename $tarball`

echo "Commercializing $tarball to produce $newtarball"

d=`basename $tarball .tar.gz`
rm -rf $d
tar xpzf $tarball

find $d -type f -print | while read name; do
    sed -e '/^ [*] This program is free software; you can redistribute it and\/or modify$/,/ [*] Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA/c\
 * See the file COPYING for license information.' $name > ${name}.tmp
    chmod --reference=$name ${name}.tmp
    touch --reference=$name ${name}.tmp
    mv ${name}.tmp $name
done

for name in $d/tools/fftw-wisdom.c; do
    sed -e '/This program is free software; you can redistribute it and\/or modify/,/["]Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA..["]/c\
"See the file COPYING for license information.\\n"' $name > ${name}.tmp
    chmod --reference=$name ${name}.tmp
    touch --reference=$name ${name}.tmp
    mv ${name}.tmp $name
done

for name in $d/configure.ac; do
    sed -e 's+genfft-k7/Makefile++' $name > ${name}.tmp
    chmod --reference=$name ${name}.tmp
    touch --reference=$name ${name}.tmp
    mv ${name}.tmp $name
done

rm -rf $d/genfft-k7
(
 cd $d; 
 autoconf; 

 # WTF?
 rm -rf autom4te.cache
)

cat > $d/COPYING <<EOF
This package is licensed commercially by the MIT Technology Licensing
Office (TLO); you should have a license agreement describing the terms
you have negotiated.

Please note that this package is provided WITHOUT ANY WARRANTY.
See your license agreement for complete details.

Contact Ms. Magdalen Christian (mchristi@mit.edu) for more information
regarding licensing.
EOF

d2=`basename $newtarball .tar.gz`
rm -rf $d2
mv $d $d2
tar cf - $d2 | gzip -9 > $newtarball
# rm -f $d2
