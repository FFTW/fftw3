rm -rf mingw64

rm -rf double-mingw64
mkdir double-mingw64
cd double-mingw64
../configure --prefix=`pwd`/../mingw64 --host=x86_64-w64-mingw32 --with-gcc-arch=nocona --enable-portable-binary --disable-alloca --with-our-malloc16 --with-windows-f77-mangling --enable-shared --disable-static --enable-threads --with-combined-threads --enable-sse2 && make -j4 && make install
cp -f tests/.libs/bench.exe `pwd`/../mingw64/bin/bench.exe
cd ..

rm -rf single-mingw64
mkdir single-mingw64
cd single-mingw64
../configure --prefix=`pwd`/../mingw64 --host=x86_64-w64-mingw32 --with-gcc-arch=nocona --enable-portable-binary --disable-alloca --with-our-malloc16 --with-windows-f77-mangling --enable-shared --disable-static --enable-threads --with-combined-threads --enable-sse --enable-float && make -j4 && make install
cp -f tests/.libs/bench.exe `pwd`/../mingw64/bin/benchf.exe
cd ..

rm -rf ldouble-mingw
mkdir ldouble-mingw
cd ldouble-mingw
../configure --prefix=`pwd`/../mingw64 --host=x86_64-w64-mingw32 --with-gcc-arch=nocona --enable-portable-binary --disable-alloca --with-our-malloc16 --with-windows-f77-mangling --enable-shared --disable-static --enable-threads --with-combined-threads --enable-long-double && make -j4 && make install
cp -f tests/.libs/bench.exe `pwd`/../mingw64/bin/benchl.exe
cd ..

cd mingw64/bin
for dll in *.dll; do
    def=`basename $dll .dll`.def
    echo "LIBRARY $dll" > $def
    echo EXPORTS >> $def
    x86_64-w64-mingw32-nm $dll | grep ' T _' | sed 's/.* T _//' | grep fftw >> $def
done
cd ../..

perl -pi -e 's,^ * #define FFTW_DLL,*/\n#define FFTW_DLL\n/*,' mingw64/include/fftw3.h

cat > README-WINDOWS <<EOF
This .zip archive contains DLL libraries and the associated header (.h)
and module-definition (.def) files of FFTW compiled for Win32.  It
also contains the corresponding bench.exe test/benchmark programs
and wisdom utilities.

There are three libraries: single precision (float), double precision,
and extended precision (long double).  To use the third library,
your compiler must have sizeof(long double) == 12.

In order to link to these .dll files from Visual C++, you need to
create .lib "import libraries" for them, and can do so with the "lib"
command that comes with VC++.  In particular, run:
     lib /machine:i386 /def:libfftw3-3.def
     lib /machine:i386 /def:libfftw3f-3.def
     lib /machine:i386 /def:libfftw3l-3.def

The single- and double-precision libraries use SSE and SSE2, respectively,
but should also work on older processors (the library checks at runtime
to see whether SSE/SSE2 is supported and disables the relevant code if not).

They were compiled by the GNU C compiler for MinGW, specifically:
EOF
x86_64-w64-mingw32-gcc --version |head -1 >> README-WINDOWS
cp -f tests/README README-bench

fftw_vers=`grep PACKAGE_VERSION double-mingw64/config.h |cut -d" " -f3 |tr -d \"`
zip=fftw-${fftw_vers}-dll64.zip
rm -f $zip
zip -vj $zip mingw64/bin/*.dll mingw64/bin/*.exe
zip -vjgl $zip mingw64/bin/*.def mingw64/include/* README COPYING COPYRIGHT NEWS README-WINDOWS README-bench
