cmake_minimum_required(VERSION 3.20)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(stlpp_examples LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    find_package(stlpp REQUIRED CONFIG)
endif()

add_executable(stlpp_examples
    src/main.cpp
    src/demos.cpp)
target_include_directories(stlpp_examples PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(stlpp_examples PRIVATE stlpp::stlpp)
target_compile_features(stlpp_examples PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(stlpp_examples PRIVATE /W4 /permissive-)
else()
    target_compile_options(stlpp_examples PRIVATE -Wall -Wextra -Wpedantic)
endif()
