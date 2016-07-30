set -e

confflags="--prefix=`pwd`/mingw64 --host=x86_64-w64-mingw32 --disable-alloca --with-our-malloc --with-windows-f77-mangling --enable-shared --disable-static --enable-threads --with-combined-threads"
export ENVIRONMENT_LIBFFTW3_LDFLAGS='-Wc,-static-libgcc'

rm -rf mingw64

(
    rm -rf double-mingw64
    mkdir double-mingw64
    cd double-mingw64
    ../configure ${confflags} --enable-sse2 --enable-avx && make -j4 && make install
    cp -f tests/.libs/bench.exe `pwd`/../mingw64/bin/bench.exe
)

(
    rm -rf single-mingw64
    mkdir single-mingw64
    cd single-mingw64
    ../configure ${confflags} --enable-sse2 --enable-avx --enable-float && make -j4 && make install
    cp -f tests/.libs/bench.exe `pwd`/../mingw64/bin/benchf.exe
)

(
    rm -rf ldouble-mingw64
    mkdir ldouble-mingw64
    cd ldouble-mingw64
    ../configure ${confflags} --enable-long-double && make -j4 && make install
    cp -f tests/.libs/bench.exe `pwd`/../mingw64/bin/benchl.exe
)

(
cd mingw64/bin
for dll in *.dll; do
    def=`basename $dll .dll`.def
    echo "LIBRARY $dll" > $def
    echo EXPORTS >> $def

# For some reason mingw does not prepend an underscore anymore in
# 64-bit mode.  Hmmm...
#    x86_64-w64-mingw32-nm $dll | grep ' T _' | sed 's/.* T _//' | grep fftw >> $def

    x86_64-w64-mingw32-nm $dll | grep ' T ' | sed 's/.* T //' | grep fftw >> $def
done
)

perl -pi -e 's,^ * #define FFTW_DLL,*/\n#define FFTW_DLL\n/*,' mingw64/include/fftw3.h

cat > README-WINDOWS <<EOF
This .zip archive contains DLL libraries and the associated header (.h)
and module-definition (.def) files of FFTW compiled for Win64.  It
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

On Visual Studio 2008 in 64-bit mode, and possibly in
other cases, you may need to specify the machine explicitly:

     lib /machine:x64 /def:libfftw3f-3.def
     lib /machine:x64 /def:libfftw3-3.def
     lib /machine:x64 /def:libfftw3l-3.def

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
