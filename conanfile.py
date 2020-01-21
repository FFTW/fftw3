from conans import ConanFile, CMake

class fftw3(ConanFile):
    name = "fftw3"
    version = "3.3.8"
    homepage = "http://www.fftw.org/"
    license = "GPL-2.0"
    url = "https://vgit.iaf.fhg.de/A3/SY/MMW_1_0/fftw"
    description = "FFTW3 library"
    settings = "os", "compiler", "build_type", "arch"
    exports_sources = "*"
    generators = "cmake_paths"
    options = {
        "shared": [True, False],
        "pic": [True, False],
        "openmp": [True, False],
        "pthread": [True, False],
        "combined_threads": [True, False],
        "precision": ["float", "double", "longdouble", "quad"],
        "sse": [True, False],
        "sse2": [True, False],
        "avx": [True, False],
        "avx2": [True, False],
        "fortran": [True, False]
        }
    default_options = {
        "shared": True,
        "pic": True,
        "openmp": False,
        "pthread": False,
        "combined_threads": False,
        "precision": "double",
        "sse": False,
        "sse2": False,
        "avx": False,
        "avx2": False,
        "fortran": False
        }
    
    def set_compile_options(self, cmake):
        cmake.definitions["CMAKE_POSITION_INDEPENDENT_CODE"] =  self.options.pic
        cmake.definitions["BUILD_TESTS"] = False
        cmake.definitions["ENABLE_OPENMP"] = self.options.openmp
        cmake.definitions["ENABLE_THREADS"] = self.options.pthread
        cmake.definitions["WITH_COMBINED_THREADS"] = self.options.combined_threads
        cmake.definitions["ENABLE_FLOAT"] = self.options.precision == "float"
        cmake.definitions["ENABLE_LONG_DOUBLE"] = self.options.precision == "longdouble"
        cmake.definitions["ENABLE_QUAD_PRECISION"] = self.options.precision == "quad"
        cmake.definitions["ENABLE_SSE"] = self.options.sse
        cmake.definitions["ENABLE_SSE2"] = self.options.sse2
        cmake.definitions["ENABLE_AVX"] = self.options.avx
        cmake.definitions["ENABLE_AVX2"] = self.options.avx2
        cmake.definitions["DISABLE_FORTRAN"] = self.options.fortran
        return cmake

    def build(self):
        cmake = CMake(self)
        cmake = self.set_compile_options(cmake)
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("COPYING", dst="licenses")
        cmake = CMake(self)
        cmake.install()
