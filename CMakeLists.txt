cmake_minimum_required(VERSION 3.25)
project(stlpp VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)

option(BUILD_TESTING "Build the GoogleTest suite" ${PROJECT_IS_TOP_LEVEL})
option(STLPP_BUILD_BENCHMARKS "Build the Google Benchmark suite" OFF)
option(STLPP_BUILD_EXAMPLES "Build the interactive example runner" OFF)
option(STLPP_SANITIZE "Enable AddressSanitizer and, where available, UndefinedBehaviorSanitizer" OFF)
option(STLPP_ENABLE_IPO "Enable interprocedural optimization for compiled targets" OFF)
option(STLPP_WARNINGS_AS_ERRORS "Treat stlpp test, benchmark, and example warnings as errors" OFF)

include(CTest)

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

function(stlpp_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    if(STLPP_WARNINGS_AS_ERRORS)
        set_property(TARGET ${target} PROPERTY COMPILE_WARNING_AS_ERROR ON)
    endif()
endfunction()

function(stlpp_enable_sanitizer_instrumentation target)
    if(NOT STLPP_SANITIZE)
        return()
    endif()

    if(MSVC)
        target_compile_options(${target} PRIVATE /fsanitize=address)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(${target} PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer)
    else()
        message(FATAL_ERROR
            "STLPP_SANITIZE is not supported by ${CMAKE_CXX_COMPILER_ID}")
    endif()
endfunction()

function(stlpp_enable_sanitizers target)
    stlpp_enable_sanitizer_instrumentation(${target})

    if(NOT STLPP_SANITIZE)
        return()
    endif()

    if(MSVC)
        # Instrumented objects select the ASan runtime automatically. link.exe
        # does not accept the compiler's /fsanitize option.
        target_link_options(${target} PRIVATE /INCREMENTAL:NO)
    else()
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endif()
endfunction()

function(stlpp_deploy_msvc_asan_runtime target)
    if(NOT (STLPP_SANITIZE AND MSVC))
        return()
    endif()

    get_filename_component(_stlpp_msvc_bin_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_stlpp_asan_arch x86_64)
    else()
        set(_stlpp_asan_arch i386)
    endif()
    set(_stlpp_asan_runtime
        "${_stlpp_msvc_bin_dir}/clang_rt.asan_dynamic-${_stlpp_asan_arch}.dll")

    if(NOT EXISTS "${_stlpp_asan_runtime}")
        message(FATAL_ERROR
            "MSVC AddressSanitizer runtime not found: ${_stlpp_asan_runtime}")
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${_stlpp_asan_runtime}"
            "$<TARGET_FILE_DIR:${target}>"
        COMMENT "Deploying the MSVC AddressSanitizer runtime for ${target}"
        VERBATIM)
endfunction()

if(STLPP_ENABLE_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(
        RESULT STLPP_IPO_SUPPORTED
        OUTPUT STLPP_IPO_ERROR
        LANGUAGES CXX)
    if(NOT STLPP_IPO_SUPPORTED)
        message(FATAL_ERROR
            "STLPP_ENABLE_IPO was requested, but IPO is unavailable: ${STLPP_IPO_ERROR}")
    endif()
endif()

function(stlpp_enable_ipo target)
    if(STLPP_ENABLE_IPO)
        set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
    endif()
endfunction()

if(BUILD_TESTING OR STLPP_BUILD_BENCHMARKS)
    include(FetchContent)
endif()

if(BUILD_TESTING)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
        URL_HASH SHA256=f179ec217f9b3b3f3c6e8b02d3e7eda997b49e4ce26d6b235c9053bec9c0bf9f
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # MSVC's annotated STL requires every linked static library to use the
    # same ASan instrumentation setting. Instrumenting dependencies also gives
    # GCC and Clang complete sanitizer coverage.
    foreach(_target gtest gtest_main gmock gmock_main)
        if(TARGET ${_target})
            stlpp_enable_sanitizer_instrumentation(${_target})
        endif()
    endforeach()

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
        test/generic_compare_test.cpp
        test/generic_copy_rotate_test.cpp
        test/generic_partition_sort_test.cpp
        test/generic_adaptive_test.cpp
        test/generic_selection_orbit_test.cpp
        test/generic_bifurcate_test.cpp
        test/generic_algorithms_test.cpp
        test/generic_iterators_test.cpp
        test/generic_parallel_test.cpp
        test/generic_containers_test.cpp
        test/generic_extensions_test.cpp
        test/generic_random_test.cpp
        test/generic_fp_test.cpp)
    target_link_libraries(stlpp_tests PRIVATE stlpp GTest::gtest_main)
    stlpp_enable_warnings(stlpp_tests)
    stlpp_enable_sanitizers(stlpp_tests)
    stlpp_deploy_msvc_asan_runtime(stlpp_tests)
    stlpp_enable_ipo(stlpp_tests)
    gtest_discover_tests(stlpp_tests)
endif()

if(STLPP_BUILD_BENCHMARKS)
    FetchContent_Declare(
        benchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.9.1.zip
        URL_HASH SHA256=8a63c9c6adf9e7ce8d0d81f251c47de83efb5e077e147d109fa2045daac8368b
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_LTO ${STLPP_ENABLE_IPO} CACHE BOOL "" FORCE)

    # An extracted release archive has no .git directory. Prevent benchmark's
    # version probe from walking up into stlpp's repository and using its tag.
    set(_stlpp_saved_disable_find_git "${CMAKE_DISABLE_FIND_PACKAGE_Git}")
    set(_stlpp_saved_git_executable "${GIT_EXECUTABLE}")
    set(CMAKE_DISABLE_FIND_PACKAGE_Git ON)
    set(GIT_EXECUTABLE "")
    FetchContent_MakeAvailable(benchmark)
    set(CMAKE_DISABLE_FIND_PACKAGE_Git "${_stlpp_saved_disable_find_git}")
    set(GIT_EXECUTABLE "${_stlpp_saved_git_executable}")
    unset(_stlpp_saved_disable_find_git)
    unset(_stlpp_saved_git_executable)

    foreach(_target benchmark benchmark_main)
        if(TARGET ${_target})
            stlpp_enable_sanitizer_instrumentation(${_target})
        endif()
    endforeach()

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
    stlpp_enable_warnings(stlpp_bench)
    stlpp_enable_sanitizers(stlpp_bench)
    stlpp_deploy_msvc_asan_runtime(stlpp_bench)
    stlpp_enable_ipo(stlpp_bench)
endif()

if(STLPP_BUILD_EXAMPLES AND PROJECT_IS_TOP_LEVEL)
    add_subdirectory(examples)
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
