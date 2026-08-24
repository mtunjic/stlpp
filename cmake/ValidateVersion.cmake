cmake_minimum_required(VERSION 3.25)

function(stlpp_check_define content name expected)
    string(REGEX MATCH
        "#define[ \t]+${name}[ \t]+([^ \t\r\n]+)"
        match "${content}")
    if(NOT match)
        message(FATAL_ERROR "Missing public version macro ${name}")
    endif()
    if(NOT CMAKE_MATCH_1 STREQUAL expected)
        message(FATAL_ERROR
            "${name} is ${CMAKE_MATCH_1}, but VERSION requires ${expected}")
    endif()
endfunction()

get_filename_component(STLPP_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
include("${CMAKE_CURRENT_LIST_DIR}/StlppVersion.cmake")
stlpp_read_version(STLPP_VERSION "${STLPP_ROOT}/VERSION")
stlpp_version_components("${STLPP_VERSION}"
    STLPP_VERSION_MAJOR STLPP_VERSION_MINOR STLPP_VERSION_PATCH)

file(READ "${STLPP_ROOT}/ports/stlpp/vcpkg.json" STLPP_VCPKG_MANIFEST)
string(JSON STLPP_VCPKG_VERSION ERROR_VARIABLE STLPP_JSON_ERROR
    GET "${STLPP_VCPKG_MANIFEST}" version)
if(STLPP_JSON_ERROR)
    message(FATAL_ERROR "Cannot read the vcpkg manifest version: ${STLPP_JSON_ERROR}")
endif()
if(NOT STLPP_VCPKG_VERSION STREQUAL STLPP_VERSION)
    message(FATAL_ERROR
        "VERSION is ${STLPP_VERSION}, but ports/stlpp/vcpkg.json is ${STLPP_VCPKG_VERSION}")
endif()

file(READ "${STLPP_ROOT}/algorithms/stlpp.h" STLPP_PUBLIC_HEADER)
stlpp_check_define("${STLPP_PUBLIC_HEADER}" STLPP_VERSION_MAJOR "${STLPP_VERSION_MAJOR}")
stlpp_check_define("${STLPP_PUBLIC_HEADER}" STLPP_VERSION_MINOR "${STLPP_VERSION_MINOR}")
stlpp_check_define("${STLPP_PUBLIC_HEADER}" STLPP_VERSION_PATCH "${STLPP_VERSION_PATCH}")
stlpp_check_define("${STLPP_PUBLIC_HEADER}" STLPP_VERSION "\"${STLPP_VERSION}\"")

if(DEFINED STLPP_GIT_TAG AND NOT STLPP_GIT_TAG STREQUAL "")
    string(REGEX REPLACE "^refs/tags/" "" STLPP_TAG "${STLPP_GIT_TAG}")
    set(STLPP_EXPECTED_TAG "v${STLPP_VERSION}")
    if(NOT STLPP_TAG STREQUAL STLPP_EXPECTED_TAG)
        message(FATAL_ERROR
            "Release tag '${STLPP_TAG}' does not match VERSION '${STLPP_EXPECTED_TAG}'")
    endif()
endif()

message(STATUS "stlpp version metadata is consistent: ${STLPP_VERSION}")