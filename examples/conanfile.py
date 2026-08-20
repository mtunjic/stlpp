import os

from conan import ConanFile
from conan.tools.build import can_run, check_min_cppstd
from conan.tools.cmake import CMake, cmake_layout


class StlppExamplesConan(ConanFile):
    """Skeleton consumer app: interactive STL++ demos.

    From this directory (after packaging or editable stlpp):

        conan install . -of build --build=missing -s compiler.cppstd=20
        cmake --preset conan-release   # or configure with the generated toolchain
        cmake --build build
        ./build/stlpp_examples
    """

    name = "stlpp_examples"
    version = "0.1.0"
    package_type = "application"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    exports_sources = "CMakeLists.txt", "src/*"

    def validate(self):
        check_min_cppstd(self, 20)

    def requirements(self):
        # Prefer an editable or just-created local package of the same version.
        self.requires("stlpp/0.1.0")

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def test(self):
        if can_run(self):
            cmd = os.path.join(self.cpp.build.bindirs[0], "stlpp_examples")
            self.run(f"{cmd} orbit", env="conanrun")
