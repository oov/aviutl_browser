# Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

# OVERVIEW
#
# CMake is a cross-platform open-source build system that can generate project
# files in many different formats. It can be downloaded from
# http://www.cmake.org or installed via a platform package manager.
#
# CMake-generated project formats that have been tested with this CEF binary
# distribution include:
#
# Linux:      Ninja, GCC 7.5.0+, Unix Makefiles
# MacOS:      Ninja, Xcode 12.2 to 13.0
# Windows:    Ninja, Visual Studio 2019+
#
# Ninja is a cross-platform open-source tool for running fast builds using
# pre-installed platform toolchains (GNU, clang, Xcode or MSVC). It can be
# downloaded from http://martine.github.io/ninja/ or installed via a platform
# package manager.
#
# CMAKE STRUCTURE
#
# This CEF binary distribution includes the following CMake files:
#
# CMakeLists.txt              Bootstrap that sets up the CMake environment.
# cmake/*.cmake               CEF configuration files shared by all targets.
# libcef_dll/CMakeLists.txt   Defines the libcef_dll_wrapper target.
# tests/*/CMakeLists.txt      Defines the test application target.
#
# See the "TODO:" comments below for guidance on how to integrate this CEF
# binary distribution into a new or existing CMake project.
#
# BUILD REQUIREMENTS
#
# The below requirements must be met to build this CEF binary distribution.
#
# - CMake version 3.19 or newer.
#
# - Linux requirements:
#   Currently supported distributions include Debian 10 (Buster), Ubuntu 18
#   (Bionic Beaver), and related. Ubuntu 18.04 64-bit with GCC 7.5.0+ is
#   recommended. Newer versions will likely also work but may not have been
#   tested.
#   Required packages include:
#     build-essential
#     libgtk3.0-dev     (required by the cefclient target only)
#
# - MacOS requirements:
#   Xcode 12.2 to 13.0 building on MacOS 10.15.4 (Catalina) or newer. Only
#   64-bit builds are supported. The Xcode command-line tools must also be
#   installed. Newer Xcode versions may not have been been tested and are not
#   recommended.
#
# - Windows requirements:
#   Visual Studio 2019 or newer building on Windows 7 or newer. Windows 10
#   64-bit is recommended. Newer versions will likely also work but may not have
#   been tested.
#
# BUILD EXAMPLES
#
# The below commands will generate project files and create a Debug build of all
# CEF targets using CMake and the platform toolchain.
#
# Start by creating and entering the CMake build output directory:
# > cd path/to/cef_binary_*
# > mkdir build && cd build
#
# To perform a Linux build using a 32-bit CEF binary distribution on a 32-bit
# Linux platform or a 64-bit CEF binary distribution on a 64-bit Linux platform:
#   Using Unix Makefiles:
#     > cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug ..
#     > make -j4 cefclient cefsimple
#
#   Using Ninja:
#     > cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Debug ..
#     > ninja cefclient cefsimple
#
# To perform a MacOS build using a 64-bit CEF binary distribution:
#   Using the Xcode IDE:
#     > cmake -G "Xcode" -DPROJECT_ARCH="x86_64" ..
#     Open build\cef.xcodeproj in Xcode and select Product > Build.
#
#   Using Ninja:
#     > cmake -G "Ninja" -DPROJECT_ARCH="x86_64" -DCMAKE_BUILD_TYPE=Debug ..
#     > ninja cefclient cefsimple
#
# To perform a MacOS build using an ARM64 CEF binary distribution:
#   Using the Xcode IDE:
#     > cmake -G "Xcode" -DPROJECT_ARCH="arm64" ..
#     Open build\cef.xcodeproj in Xcode and select Product > Build.
#
#   Using Ninja:
#     > cmake -G "Ninja" -DPROJECT_ARCH="arm64" -DCMAKE_BUILD_TYPE=Debug ..
#     > ninja cefclient cefsimple
#
# To perform a Windows build using a 32-bit CEF binary distribution:
#   Using the Visual Studio 2019 IDE:
#     > cmake -G "Visual Studio 16" -A Win32 ..
#     Open build\cef.sln in Visual Studio and select Build > Build Solution.
#
#   Using Ninja with Visual Studio 2019 command-line tools:
#     (this path may be different depending on your Visual Studio installation)
#     > "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars32.bat"
#     > cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Debug ..
#     > ninja cefclient cefsimple
#
# To perform a Windows build using a 64-bit CEF binary distribution:
#   Using the Visual Studio 2019 IDE:
#     > cmake -G "Visual Studio 16" -A x64 ..
#     Open build\cef.sln in Visual Studio and select Build > Build Solution.
#
#   Using Ninja with Visual Studio 2019 command-line tools:
#     (this path may be different depending on your Visual Studio installation)
#     > "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
#     > cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Debug ..
#     > ninja cefclient cefsimple
#
# To perform a Windows build using an ARM64 CEF binary distribution:
#   Using the Visual Studio 2019 IDE:
#     > cmake -G "Visual Studio 16" -A arm64 ..
#     Open build\cef.sln in Visual Studio and select Build > Build Solution.
#
#   Using Ninja with Visual Studio 2019 command-line tools:
#     (this path may be different depending on your Visual Studio installation)
#     > "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsamd64_arm64.bat"
#     > cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Debug ..
#     > ninja cefsimple

#
# Global setup.
#

# For VS2019 and Xcode 12+ support.
cmake_minimum_required(VERSION 3.19)

# Only generate Debug and Release configuration types.
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# Project name.
# TODO: Change this line to match your project name when you copy this file.
project(aviutl_browser)

# Use folders in the resulting project files.
set_property(GLOBAL PROPERTY OS_FOLDERS ON)


#
# CEF_ROOT setup.
# This variable must be set to locate the binary distribution.

set(CEF_URL  "https://cef-builds.spotifycdn.com/cef_binary_99.2.14%2Bg3f796b8%2Bchromium-99.0.4844.84_windows64_minimal.tar.bz2")
set(CEF_ARCHIVE_PATH "${CMAKE_BINARY_DIR}/cef.tar.bz2")
set(CEF_DIR "${CMAKE_BINARY_DIR}/cef")

set(CEF_ROOT "${CEF_DIR}")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

if(NOT EXISTS "${CEF_ARCHIVE_PATH}")
  file(DOWNLOAD "${CEF_URL}" "${CEF_ARCHIVE_PATH}")
endif()

if(NOT EXISTS "${CEF_DIR}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xvzf "${CEF_ARCHIVE_PATH}" WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
  file(GLOB old_dir LIST_DIRECTORIES true "${CMAKE_BINARY_DIR}/cef_binary_*")
  file(RENAME "${old_dir}" "${CEF_DIR}")
endif()

set(EMOJI_VERSION "v0.0.2")
set(EMOJI_URL "https://github.com/oov/aviutl_browser_emoji/releases/download/${EMOJI_VERSION}/aviutl_browser_emoji_${EMOJI_VERSION}.zip")
set(EMOJI_PATH "${CMAKE_CURRENT_BINARY_DIR}/emoji.zip")
set(EMOJI_DIR "${CMAKE_CURRENT_BINARY_DIR}/emoji")
if(NOT EXISTS "${EMOJI_PATH}")
  file(DOWNLOAD "${EMOJI_URL}" "${EMOJI_PATH}")
endif()
if(NOT EXISTS "${EMOJI_DIR}")
  file(MAKE_DIRECTORY "${EMOJI_DIR}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xvzf "${EMOJI_PATH}" WORKING_DIRECTORY "${EMOJI_DIR}")
endif()

set(MARKDOWN_VERSION "v0.0.3")
set(MARKDOWN_URL "https://github.com/oov/aviutl_browser_markdown/releases/download/${MARKDOWN_VERSION}/aviutl_browser_markdown_${MARKDOWN_VERSION}.zip")
set(MARKDOWN_PATH "${CMAKE_CURRENT_BINARY_DIR}/markdown.zip")
set(MARKDOWN_DIR "${CMAKE_CURRENT_BINARY_DIR}/markdown")
if(NOT EXISTS "${MARKDOWN_PATH}")
  file(DOWNLOAD "${MARKDOWN_URL}" "${MARKDOWN_PATH}")
endif()
if(NOT EXISTS "${MARKDOWN_DIR}")
  file(MAKE_DIRECTORY "${MARKDOWN_DIR}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xvzf "${MARKDOWN_PATH}" WORKING_DIRECTORY "${MARKDOWN_DIR}")
endif()

add_custom_target(copy_script
  COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/README.md" "${PROJECT_BINARY_DIR}/bin/aviutl_browser.txt"
  COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/LICENSES.chromium.html" "${PROJECT_BINARY_DIR}/bin/LICENSES.chromium.html"
  COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/src/lua/browser.lua" "${PROJECT_BINARY_DIR}/bin/script/browser/browser.lua"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${EMOJI_DIR}" "${PROJECT_BINARY_DIR}/bin"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${MARKDOWN_DIR}" "${PROJECT_BINARY_DIR}/bin"
)

#
# Load the CEF configuration.
#

# Execute FindCEF.cmake which must exist in CMAKE_MODULE_PATH.
find_package(CEF REQUIRED)


#
# Define CEF-based targets.
#

# Include the libcef_dll_wrapper target.
# Comes from the libcef_dll/CMakeLists.txt file in the binary distribution
# directory.
add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)

add_subdirectory(src)

# Display configuration settings.
PRINT_CEF_CONFIG()

