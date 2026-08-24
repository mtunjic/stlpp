cmake_minimum_required(VERSION 3.25)

function(stlpp_replace_define content_variable output name value)
    set(content "${${content_variable}}")
    string(REGEX MATCHALL
        "#define[ \t]+${name}[ \t]+[^ \t\r\n]+"
        fields "${content}")
    list(LENGTH fields field_count)
    if(NOT field_count EQUAL 1)
        message(FATAL_ERROR "Expected exactly one public version macro ${name}")
    endif()
    string(REGEX REPLACE
        "#define[ \t]+${name}[ \t]+[^ \t\r\n]+"
        "#define ${name} ${value}"
        updated "${content}")
    set(${output} "${updated}" PARENT_SCOPE)
endfunction()

if(NOT DEFINED STLPP_VERSION OR STLPP_VERSION STREQUAL "")
    message(FATAL_ERROR
        "Usage: cmake -DSTLPP_VERSION=<major.minor.patch> -P cmake/SetVersion.cmake")
endif()

get_filename_component(STLPP_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
include("${CMAKE_CURRENT_LIST_DIR}/StlppVersion.cmake")
stlpp_validate_version("${STLPP_VERSION}" "-DSTLPP_VERSION")
stlpp_version_components("${STLPP_VERSION}"
    STLPP_VERSION_MAJOR STLPP_VERSION_MINOR STLPP_VERSION_PATCH)

set(STLPP_VCPKG_PATH "${STLPP_ROOT}/ports/stlpp/vcpkg.json")
file(READ "${STLPP_VCPKG_PATH}" STLPP_VCPKG_MANIFEST)
string(REGEX MATCHALL
    "\"version\"[ \t\r\n]*:[ \t\r\n]*\"[^\"]+\""
    STLPP_VERSION_FIELDS "${STLPP_VCPKG_MANIFEST}")
list(LENGTH STLPP_VERSION_FIELDS STLPP_VERSION_FIELD_COUNT)
if(NOT STLPP_VERSION_FIELD_COUNT EQUAL 1)
    message(FATAL_ERROR
        "Expected exactly one version field in ${STLPP_VCPKG_PATH}")
endif()
string(REGEX REPLACE
    "\"version\"[ \t\r\n]*:[ \t\r\n]*\"[^\"]+\""
    "\"version\": \"${STLPP_VERSION}\""
    STLPP_UPDATED_VCPKG_MANIFEST "${STLPP_VCPKG_MANIFEST}")

set(STLPP_HEADER_PATH "${STLPP_ROOT}/algorithms/stlpp.h")
file(READ "${STLPP_HEADER_PATH}" STLPP_UPDATED_HEADER)
stlpp_replace_define(STLPP_UPDATED_HEADER STLPP_UPDATED_HEADER
    STLPP_VERSION_MAJOR "${STLPP_VERSION_MAJOR}")
stlpp_replace_define(STLPP_UPDATED_HEADER STLPP_UPDATED_HEADER
    STLPP_VERSION_MINOR "${STLPP_VERSION_MINOR}")
stlpp_replace_define(STLPP_UPDATED_HEADER STLPP_UPDATED_HEADER
    STLPP_VERSION_PATCH "${STLPP_VERSION_PATCH}")
stlpp_replace_define(STLPP_UPDATED_HEADER STLPP_UPDATED_HEADER
    STLPP_VERSION "\"${STLPP_VERSION}\"")

file(WRITE "${STLPP_ROOT}/VERSION" "${STLPP_VERSION}\n")
file(WRITE "${STLPP_VCPKG_PATH}" "${STLPP_UPDATED_VCPKG_MANIFEST}")
file(WRITE "${STLPP_HEADER_PATH}" "${STLPP_UPDATED_HEADER}")
message(STATUS "Updated stlpp to ${STLPP_VERSION}")
message(STATUS "Next: cmake -P cmake/ValidateVersion.cmake")
message(STATUS "Then create annotated tag v${STLPP_VERSION}")