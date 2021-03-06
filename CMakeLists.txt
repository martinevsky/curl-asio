# curl-asio
# Seamlessly integrate libcurl with Boost.Asio
#
# Copyright (c) 2013 Oliver Kuckertz <oliver.kuckertz@mologie.de>
# See COPYING for license information.

CMAKE_MINIMUM_REQUIRED(VERSION 3.5 FATAL_ERROR)
PROJECT(curl-asio)

# Local curl and options
SET(CURL_ROOT "external/curl")
SET(BUILD_CURL_EXE OFF CACHE BOOL "")
SET(BUILD_TESTING OFF CACHE BOOL "")
SET(CURL_STATICLIB ON CACHE BOOL "")
SET(HTTP_ONLY ON CACHE BOOL "")
SET(CURL_DISABLE_COOKIES ON CACHE BOOL "")
INCLUDE_DIRECTORIES (${CURL_BINARY_DIR}/include/curl)
ADD_SUBDIRECTORY (external/curl)

# Adjust the module path
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

# Build options
OPTION(BUILD_STATIC "Build the static library" ON)
OPTION(BUILD_SHARED "Build the shared library" OFF)
OPTION(BUILD_EXAMPLES "Build the examples" ON)

IF(NOT BUILD_STATIC AND NOT BUILD_SHARED)
	MESSAGE(FATAL_ERROR "Neither BUILD_STATIC nor BUILD_SHARED is set")
ENDIF()

SET(CURLASIO_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Prefer the shared interface over the static for building examples
IF(BUILD_SHARED)
	SET(EXAMPLE_LINK_TARGET curlasio-shared)
ELSE()
	SET(EXAMPLE_LINK_TARGET curlasio-static)
	SET(EXAMPLE_PROJECT_DEFINITIONS -DCURLASIO_STATIC)
ENDIF()

IF(MSVC)
	# Select Windows version
	OPTION(ENABLE_VISTA_FEATURES "Enable Vista features. This disables some workarounds, but sacrifices Windows XP compatibility." ON)
	IF(ENABLE_VISTA_FEATURES)
		SET(TARGET_WINDOWS_VERSION 0x0600)
	ELSE()
		SET(TARGET_WINDOWS_VERSION 0x0501)
	ENDIF()
	ADD_DEFINITIONS(-D_WIN32_WINNT=${TARGET_WINDOWS_VERSION})
	
	# Disable using __declspec(dllimport) when linking statically
	OPTION(CURL_IS_STATIC "Check if libcurl is a static library" ON)
	IF(CURL_IS_STATIC)
		ADD_DEFINITIONS(-DCURL_STATICLIB)
	ENDIF()
ENDIF()

# Uncomment for extended events tracing to cerr (Notice: Very noisy)
# ADD_DEFINITIONS(-DBOOST_ASIO_ENABLE_HANDLER_TRACKING)

# Find dependencies
FIND_PACKAGE(CURLASIO-CURL REQUIRED)
#FIND_PACKAGE(Boost REQUIRED COMPONENTS chrono date_time system thread)

ADD_DEFINITIONS(-DASIO_STANDALONE)
INCLUDE_DIRECTORIES (${CMAKE_CURRENT_SOURCE_DIR}/external/asio/asio/include)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# Move along
ADD_SUBDIRECTORY(src)
IF(BUILD_EXAMPLES)
	ADD_SUBDIRECTORY(examples)
ENDIF()

# Install CMake modules for curl-asio and libcurl
OPTION(INSTALL_CMAKE_MODULES "Install CMake module files for libcurl and curl-asio" ON)
IF(INSTALL_CMAKE_MODULES)
	IF(APPLE)
		IF((IS_SYMLINK "${CMAKE_INSTALL_PREFIX}/share/cmake") OR (IS_SYMLINK "${CMAKE_INSTALL_PREFIX}/share/cmake/Modules"))
			MESSAGE(FATAL_ERROR "The destination path `${CMAKE_INSTALL_PREFIX}/share/cmake' is a symlink. "
				"If you are using Homebrew, run the following set of commands prior to installing this library:\n"
				"    brew unlink cmake\n"
				"    mkdir -p '${CMAKE_INSTALL_PREFIX}/share/cmake/Modules'\n"
				"    brew link cmake\n"
				"Without these changes, updating CMake through Homebrew would break the installation. Alternatively, choose a different CMAKE_INSTALL_PREFIX.")
		ENDIF()
	ENDIF()
	INSTALL(DIRECTORY cmake/ DESTINATION share/cmake/Modules)
ENDIF()
