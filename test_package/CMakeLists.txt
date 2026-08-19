cmake_minimum_required(VERSION 3.20)
project(stlpp_test_package LANGUAGES CXX)

find_package(stlpp REQUIRED)

add_executable(example src/example.cpp)
target_link_libraries(example PRIVATE stlpp::stlpp)
target_compile_features(example PRIVATE cxx_std_20)
