# Copyright (c) 2016 Michael Hansen
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(string_theory)

# We will detect and use optional features in newer C++ standards,
# but C++11 is required at minimum
if(${CMAKE_VERSION} VERSION_GREATER "3.11.99")
    set(CMAKE_CXX_STANDARD 20)
elseif(${CMAKE_VERSION} VERSION_GREATER "3.7.99")
    set(CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_STANDARD 14)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

if(POLICY CMP0067)
    # Honor CMAKE_CXX_STANDARD in try_compile() commands
    cmake_policy(SET CMP0067 NEW)
endif()

set(ST_MAJOR_VERSION 3)
set(ST_MINOR_VERSION 0)
set(ST_VERSION ${ST_MAJOR_VERSION}.${ST_MINOR_VERSION})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES ".*Clang")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra ${CMAKE_CXX_FLAGS}")

    option(ST_BUILD_TEST_COVERAGE "Enable code coverage in string_theory and tests" OFF)
    if(ST_BUILD_TEST_COVERAGE)
        include(CodeCoverage)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -fprofile-arcs -ftest-coverage")
    endif()
endif()

try_compile(ST_HAVE_CXX20_CHAR8_TYPES "${PROJECT_BINARY_DIR}"
            "${PROJECT_SOURCE_DIR}/cmake/check_char8_types.cpp")
try_compile(ST_HAVE_INT64 "${PROJECT_BINARY_DIR}"
            "${PROJECT_SOURCE_DIR}/cmake/check_int64.cpp")
try_compile(ST_HAVE_DEPRECATED_ATTR "${PROJECT_BINARY_DIR}"
            "${PROJECT_SOURCE_DIR}/cmake/check_deprecated_attr.cpp")

try_compile(ST_INT64_IS_NOT_LONG "${PROJECT_BINARY_DIR}"
            "${PROJECT_SOURCE_DIR}/cmake/check_int64_is_long.cpp")
if(NOT ST_INT64_IS_NOT_LONG)
    set(ST_INT64_IS_LONG TRUE)
endif()

try_compile(ST_HAVE_CXX17_STRING_VIEW "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/cmake/check_string_view.cpp")
try_compile(ST_HAVE_EXPERIMENTAL_STRING_VIEW "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/cmake/check_string_view.cpp"
    COMPILE_DEFINITIONS "-DTRY_EXPERIMENTAL_STRING_VIEW")

if(CMAKE_COMPILER_IS_GNUCXX)
    set(ST_CXXFS_LIBS stdc++fs)
endif()
try_compile(ST_HAVE_CXX17_FILESYSTEM "${PROJECT_BINARY_DIR}"
            "${PROJECT_SOURCE_DIR}/cmake/check_filesystem.cpp"
            LINK_LIBRARIES ${ST_CXXFS_LIBS})
try_compile(ST_HAVE_EXPERIMENTAL_FILESYSTEM "${PROJECT_BINARY_DIR}"
            "${PROJECT_SOURCE_DIR}/cmake/check_filesystem.cpp"
            COMPILE_DEFINITIONS "-DTRY_EXPERIMENTAL_FILESYSTEM"
            LINK_LIBRARIES ${ST_CXXFS_LIBS})

option(ST_BUILD_STATIC "Build string_theory as a static library" OFF)
if(ST_BUILD_STATIC)
    set(ST_LIB_TYPE STATIC)
    set(ST_BUILD_SHARED FALSE)
else()
    set(ST_LIB_TYPE SHARED)
    set(ST_BUILD_SHARED TRUE)
endif()

option(ST_NO_STL_STRINGS "Disable std::string and std::filesystem::path support" OFF)

configure_file("${PROJECT_SOURCE_DIR}/include/st_config.h.in"
               "${PROJECT_BINARY_DIR}/include/st_config.h")
include_directories("${PROJECT_BINARY_DIR}/include")

set(ST_HEADERS_PRIV
    include/st_assert.h
    include/st_charbuffer.h
    include/st_codecs.h
    include/st_format.h
    include/st_format_numeric.h
    include/st_formatter.h
    include/st_iostream.h
    include/st_stdio.h
    include/st_string.h
    include/st_string_helpers.h
    include/st_stringstream.h
    include/st_utf_conv.h
    "${PROJECT_BINARY_DIR}/include/st_config.h"
)
set(ST_HEADERS_PUB
    include/string_theory/assert
    include/string_theory/char_buffer
    include/string_theory/codecs
    include/string_theory/exceptions
    include/string_theory/formatter
    include/string_theory/format
    include/string_theory/iostream
    include/string_theory/stdio
    include/string_theory/string
    include/string_theory/string_stream
    include/string_theory/utf_conversion
)

set(ST_SOURCES
    src/st_assert.cpp
    src/st_codecs.cpp
    src/st_format.cpp
    src/st_string.cpp
    src/st_utf_conv.cpp
)

set(ST_INSTALL_BIN_DIR "bin" CACHE PATH "Path to install DLLs on Windows")
set(ST_INSTALL_LIB_DIR "lib" CACHE PATH "Path to install shared libraries")
set(ST_INSTALL_INCLUDE_DIR "include" CACHE PATH "Path to install headers")
set(ST_INSTALL_CMAKE_DIR "${ST_INSTALL_LIB_DIR}/cmake" CACHE PATH "Path to install CMake files")

add_library(string_theory ${ST_LIB_TYPE}
            ${ST_HEADERS_PRIV} ${ST_SOURCES})
target_compile_definitions(string_theory PRIVATE ST_BUILD_EXPORTS)
if(WIN32)
    set_target_properties(string_theory PROPERTIES DEBUG_POSTFIX "d")
endif()

if(ST_HAVE_EXPERIMENTAL_FILESYSTEM)
    target_link_libraries(string_theory ${ST_CXXFS_LIBS})
endif()

# Note:  Using <major>.0 so the library name doens't change between minor
# releases.  ABI incompatible changes are only allowed between major version
# updates, starting with the 1.0 release.
set_target_properties(string_theory PROPERTIES
    VERSION ${ST_MAJOR_VERSION}.0
    SOVERSION ${ST_MAJOR_VERSION}
)

target_include_directories(string_theory PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

install(TARGETS string_theory
        EXPORT string_theory-targets
        RUNTIME DESTINATION "${ST_INSTALL_BIN_DIR}" COMPONENT shlib
        LIBRARY DESTINATION "${ST_INSTALL_LIB_DIR}" COMPONENT shlib
        ARCHIVE DESTINATION "${ST_INSTALL_LIB_DIR}" COMPONENT staticlib
)
install(FILES ${ST_HEADERS_PRIV} ${ST_HEADERS_PUB}
        DESTINATION "${ST_INSTALL_INCLUDE_DIR}/string_theory" COMPONENT devel)

export(TARGETS string_theory
       FILE "${PROJECT_BINARY_DIR}/string_theory-targets.cmake")

configure_file(cmake/string_theory-config.cmake.in
               "${PROJECT_BINARY_DIR}/string_theory-config.cmake" @ONLY)
configure_file(cmake/string_theory-config-version.cmake.in
               "${PROJECT_BINARY_DIR}/string_theory-config-version.cmake" @ONLY)

install(FILES
        "${PROJECT_BINARY_DIR}/string_theory-config.cmake"
        "${PROJECT_BINARY_DIR}/string_theory-config-version.cmake"
        DESTINATION "${ST_INSTALL_CMAKE_DIR}/string_theory" COMPONENT devel)
install(EXPORT string_theory-targets
        DESTINATION "${ST_INSTALL_CMAKE_DIR}/string_theory" COMPONENT devel)

option(ST_BUILD_TESTS "Build string_theory test suite (recommended)" ON)
if(ST_BUILD_TESTS)
    add_subdirectory(test)
endif()
