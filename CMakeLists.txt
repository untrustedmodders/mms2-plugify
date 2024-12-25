# mms2-plugify
# Copyright (C) 2024 untrustedmodders
# Licensed under the MIT license. See LICENSE file in the project root for details.

cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
	cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()

project(mms2-plugify
	VERSION 1.0.0.0
	DESCRIPTION "Plugify MMS2 Core"
	HOMEPAGE_URL "https://github.com/untrustedmodders/mms2-plugify"
	LANGUAGES C CXX
)

string(REGEX REPLACE "^[^-]*-" "" PROJECT_NAME_SUBSTRING "${PROJECT_NAME}")
string(TOLOWER "${PROJECT_NAME_SUBSTRING}" PROJECT_NAME_LOWER)
string(TOUPPER "${PROJECT_NAME_SUBSTRING}" PROJECT_NAME_UPPER)

set(PROJECT_OUTPUT_NAME "${PROJECT_NAME_SUBSTRING}")

set(PROJECT_AUTHOR "Untrusted Modders Team")
set(PROJECT_DESCRIPTION_FULL "Metamod:Source Plugify plugin")
set(PROJECT_LICENSE "MIT")
string(TIMESTAMP PROJECT_BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP PROJECT_BUILD_TIME "%H:%M:%S")
string(TIMESTAMP PROJECT_BUILD_DATE_YEAR "%Y")

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SYMBOLS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sym")
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

function(set_or_external_dir VAR_NAME TARGET)
	if(${VAR_NAME})
		file(TO_CMAKE_PATH "${${VAR_NAME}}" ${VAR_NAME})
	else()
		set(${VAR_NAME} "${EXTERNAL_DIR}/${TARGET}")
	endif()

	set(${VAR_NAME} "${${VAR_NAME}}" PARENT_SCOPE)
endfunction()

set_or_external_dir(METAMOD_DIR "metamod-source")
set_or_external_dir(PLUGIFY_DIR "plugify")
set_or_external_dir(SOURCESDK_DIR "sourcesdk")

include(cmake/platform/shared.cmake)
include(cmake/metamod.cmake)
include(cmake/plugify.cmake)
include(cmake/sourcesdk.cmake)

file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${SOURCE_DIR}/*.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${METAMOD_SOURCESDK_SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
		C_STANDARD 11
		C_STANDARD_REQUIRED ON
		C_EXTENSIONS OFF

		CXX_STANDARD 20
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_OUTPUT_NAME})

if(WINDOWS)
	set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(MACOS)
	set_target_properties(${PROJECT_NAME} PROPERTIES OSX_ARCHITECTURES "x86_64")
endif()

target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILER_OPTIONS} ${SOURCESDK_COMPILE_OPTIONS} ${PLUGIFY_COMPILE_OPTIONS})

if(UNIX)
	if(APPLE)
		target_link_options(${PROJECT_NAME} PRIVATE "-Wl,-exported_symbols_list,${SYMBOLS_DIR}/exported_symbols.lds")
	elseif(LINUX)
		target_link_options(${PROJECT_NAME} PRIVATE "-Wl,--version-script,${SYMBOLS_DIR}/version_script.lds")
	endif()
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE ${METAMOD_COMPILE_DEFINITIONS} ${PLUGIFY_COMPILE_DEFINITIONS})
target_include_directories(${PROJECT_NAME} PRIVATE ${METAMOD_INCLUDE_DIRS} ${PLUGIFY_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE ${PLUGIFY_BINARY_DIR} ${PLUGIFY_LINK_LIBRARIES} ${SOURCESDK_BINARY_DIR})

if(LINUX)
    set(PLUGIFY_PREFER_OWN_SYMBOLS "true")
else()
    set(PLUGIFY_PREFER_OWN_SYMBOLS "false")
endif()

configure_file(
	${CMAKE_SOURCE_DIR}/plugify.pconfig.in
	${CMAKE_BINARY_DIR}/plugify.pconfig
)

if(WINDOWS)
	set(PLUGIFY_VDF_PLATFORM "win64")
elseif(LINUX)
	set(PLUGIFY_VDF_PLATFORM "linuxsteamrt64")
elseif(APPLE)
	set(PLUGIFY_VDF_PLATFORM "osx64")
else()
	message(FATAL_ERROR "Unsupported platform")
endif()

configure_file(
	${CMAKE_SOURCE_DIR}/plugify.vdf.in
	${CMAKE_BINARY_DIR}/addons/metamod/plugify.vdf
)
