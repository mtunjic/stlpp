cmake_minimum_required(VERSION 3.20)
project(stlpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(STLPP_IS_TOPLEVEL ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
else()
    set(STLPP_IS_TOPLEVEL OFF)
endif()

option(STLPP_BUILD_TESTING "Build GoogleTest suite" ${STLPP_IS_TOPLEVEL})
option(STLPP_BUILD_BENCHMARKS "Build Google Benchmark suite" ${STLPP_IS_TOPLEVEL})
option(STLPP_SANITIZE "Build tests with ASan and UBSan (requires libasan/libubsan)" OFF)

include(GNUInstallDirs)

add_library(stlpp INTERFACE)
add_library(stlpp::stlpp ALIAS stlpp)
target_include_directories(stlpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/algorithms>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extensions>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/iterators>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/containers>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/stlpp>)
target_compile_features(stlpp INTERFACE cxx_std_20)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(stlpp INTERFACE Threads::Threads)

function(stlpp_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endfunction()

function(stlpp_sanitize target)
    if(STLPP_SANITIZE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endif()
endfunction()

if(STLPP_BUILD_TESTING OR STLPP_BUILD_BENCHMARKS)
    include(FetchContent)
    set(_gtest_zip "${CMAKE_CURRENT_SOURCE_DIR}/build/_deps_src/gtest.zip")
    set(_gbench_zip "${CMAKE_CURRENT_SOURCE_DIR}/build/_deps_src/benchmark.zip")
    if(NOT EXISTS "${_gtest_zip}")
        set(_gtest_zip "https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip")
    endif()
    if(NOT EXISTS "${_gbench_zip}")
        set(_gbench_zip "https://github.com/google/benchmark/archive/refs/tags/v1.9.1.zip")
    endif()
endif()

if(STLPP_BUILD_TESTING)
    FetchContent_Declare(
        googletest
        URL "${_gtest_zip}"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    include(GoogleTest)

    add_executable(stlpp_tests
        test/binary_counter_test.cpp
        test/split_join_test.cpp
        test/random_test.cpp
        test/core_algorithms_test.cpp
        test/extensions_test.cpp
        test/iterators_test.cpp
        test/containers_test.cpp
        test/hybrid_test.cpp
        test/fp_algorithms_test.cpp
        test/fp_iterators_test.cpp
        test/concept_axioms_test.cpp
        test/generic_algorithms_test.cpp
        test/generic_iterators_test.cpp
        test/generic_parallel_test.cpp
        test/generic_containers_test.cpp
        test/generic_extensions_test.cpp)
    target_link_libraries(stlpp_tests PRIVATE stlpp GTest::gtest_main)
    stlpp_warnings(stlpp_tests)
    stlpp_sanitize(stlpp_tests)
    gtest_discover_tests(stlpp_tests)
endif()

if(STLPP_BUILD_BENCHMARKS)
    FetchContent_Declare(
        benchmark
        URL "${_gbench_zip}"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(benchmark)

    add_executable(stlpp_bench
        test/bench_main.cpp
        test/binary_counter_bench.cpp
        test/split_join_bench.cpp
        test/random_bench.cpp
        test/core_algorithms_bench.cpp
        test/extensions_bench.cpp
        test/iterators_bench.cpp
        test/containers_bench.cpp
        test/hybrid_bench.cpp
        test/fp_bench.cpp)
    target_link_libraries(stlpp_bench PRIVATE stlpp benchmark::benchmark)
    stlpp_warnings(stlpp_bench)
endif()

install(TARGETS stlpp EXPORT stlppTargets)

set(STLPP_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/stlpp")
install(DIRECTORY algorithms/ containers/ iterators/ extensions/
    DESTINATION ${STLPP_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h")
install(FILES utils.h DESTINATION ${STLPP_INSTALL_INCLUDEDIR})
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/install_stlpp.h"
    "#pragma once\n#include \"stlpp/stlpp.h\"\n")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/install_stlpp.h"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    RENAME stlpp.h)

install(EXPORT stlppTargets
    FILE stlppTargets.cmake
    NAMESPACE stlpp::
    DESTINATION ${CMAKE_INSTALL_DATADIR}/stlpp)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/stlppConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/stlppConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/stlppConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/stlpp)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/stlppConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/stlppConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/stlpp)
install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/stlpp)
