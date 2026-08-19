set(VCPKG_BUILD_TYPE release)

get_filename_component(SOURCE_PATH "${CURRENT_PORT_DIR}/../.." ABSOLUTE)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DSTLPP_BUILD_TESTING=OFF
        -DSTLPP_BUILD_BENCHMARKS=OFF
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME stlpp CONFIG_PATH share/stlpp)
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
