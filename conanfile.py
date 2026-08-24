from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.files import copy, load, save
import os


class StlppConan(ConanFile):
    name = "stlpp"
    license = "MIT"
    author = "Marko Tunjić"
    url = "https://github.com/mtunjic/stlpp"
    homepage = "https://github.com/mtunjic/stlpp"
    description = "Header-only Stepanov-style algorithms, iterators, and containers (STL++)"
    topics = ("algorithms", "iterators", "containers", "generic-programming", "header-only")
    package_type = "header-library"
    no_copy_source = True
    settings = "os", "compiler", "build_type", "arch"
    exports = "VERSION"
    exports_sources = (
        "VERSION",
        "CMakeLists.txt",
        "cmake/*",
        "include/*.h",
        "algorithms/*.h",
        "containers/*.h",
        "iterators/*.h",
        "extensions/*.h",
        "utils.h",
        "LICENSE",
    )

    def set_version(self):
        self.version = load(self, os.path.join(self.recipe_folder, "VERSION")).strip()

    def validate(self):
        check_min_cppstd(self, 20)

    def package_id(self):
        self.info.clear()

    def layout(self):
        self.cpp.source.includedirs = [
            "include",
            "algorithms",
            "containers",
            "iterators",
            "extensions",
        ]
        self.cpp.package.includedirs = ["include"]
        self.cpp.package.bindirs = []
        self.cpp.package.libdirs = []

    def package(self):
        dest = os.path.join(self.package_folder, "include", "stlpp")
        for folder in ("algorithms", "containers", "iterators", "extensions"):
            copy(self, "*.h", os.path.join(self.source_folder, folder), dest)
        copy(self, "utils.h", self.source_folder, dest)
        save(
            self,
            os.path.join(self.package_folder, "include", "stlpp.h"),
            '#pragma once\n#include "stlpp/stlpp.h"\n',
        )
        copy(self, "LICENSE", self.source_folder, os.path.join(self.package_folder, "licenses"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "stlpp")
        self.cpp_info.set_property("cmake_target_name", "stlpp::stlpp")
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        if self.settings.get_safe("os") in ("Linux", "FreeBSD"):
            self.cpp_info.system_libs = ["pthread"]
