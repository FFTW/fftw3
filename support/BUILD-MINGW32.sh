set -e

confflags="--prefix=`pwd`/mingw32 --host=i686-w64-mingw32 --with-our-malloc --with-windows-f77-mangling --enable-shared --disable-static --enable-threads --with-combined-threads --with-incoming-stack-boundary=2"
export ENVIRONMENT_LIBFFTW3_LDFLAGS='-Wc,-static-libgcc'

rm -rf mingw32

(
    rm -rf double-mingw32
    mkdir double-mingw32
    cd double-mingw32
    ../configure ${confflags} --enable-sse2 --enable-avx && make -j4 && make install
    cp -f tests/.libs/bench.exe `pwd`/../mingw32/bin/bench.exe
)

(
    rm -rf single-mingw32
    mkdir single-mingw32
    cd single-mingw32
    ../configure ${confflags} --enable-sse --enable-avx --enable-float && make -j4 && make install
    cp -f tests/.libs/bench.exe `pwd`/../mingw32/bin/benchf.exe
)

(
    rm -rf ldouble-mingw32
    mkdir ldouble-mingw32
    cd ldouble-mingw32
    ../configure ${confflags} --enable-long-double && make -j4 && make install
    cp -f tests/.libs/bench.exe `pwd`/../mingw32/bin/benchl.exe
)

(
cd mingw32/bin
for dll in *.dll; do
    def=`basename $dll .dll`.def
    echo "LIBRARY $dll" > $def
    echo EXPORTS >> $def
    i686-w64-mingw32-nm $dll | grep ' T _' | sed 's/.* T _//' | grep fftw >> $def
done
)

perl -pi -e 's,^ * #define FFTW_DLL,*/\n#define FFTW_DLL\n/*,' mingw32/include/fftw3.h

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
     lib /def:libfftw3f-3.def
     lib /def:libfftw3-3.def
     lib /def:libfftw3l-3.def

The single- and double-precision libraries use SSE and SSE2, respectively,
but should also work on older processors (the library checks at runtime
to see whether SSE/SSE2 is supported and disables the relevant code if not).

They were compiled by the GNU C compiler for MinGW, specifically:
EOF
i686-w64-mingw32-gcc --version |head -1 >> README-WINDOWS
cp -f tests/README README-bench

fftw_vers=`grep PACKAGE_VERSION double-mingw32/config.h |cut -d" " -f3 |tr -d \"`
zip=fftw-${fftw_vers}-dll32.zip
rm -f $zip
zip -vj $zip mingw32/bin/*.dll mingw32/bin/*.exe
zip -vjgl $zip mingw32/bin/*.def mingw32/include/* README COPYING COPYRIGHT NEWS README-WINDOWS README-bench
