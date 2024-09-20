cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()

project(cs2-plugify VERSION 1.0.0.0 DESCRIPTION "Plugify CS2 Core" HOMEPAGE_URL "https://github.com/untrustedmodders/cs2-plugify" LANGUAGES C CXX ASM)

set(PROJECT_AUTHOR "Untrusted Modders Team")
set(PROJECT_DESCRIPTION_FULL "Metamod:Source Plugify plugin")
set(PROJECT_LICENSE "MTL")
string(TIMESTAMP PROJECT_BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP PROJECT_BUILD_TIME "%H:%M:%S")

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if(WIN32 AND NOT MSVC)
    message(FATAL "MSVC restricted.")
endif()

set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")

function(set_or_external_dir VAR_NAME DEFAULT_DIR)
    if(${VAR_NAME})
        file(TO_CMAKE_PATH "${${VAR_NAME}}" ${VAR_NAME})
    else()
        set(${VAR_NAME} "${EXTERNAL_DIR}/${DEFAULT_DIR}")
    endif()

    set(${VAR_NAME} "${${VAR_NAME}}" PARENT_SCOPE)
endfunction()

string(TIMESTAMP PROJECT_BUILD_DATE_YEAR "%Y")

set_or_external_dir(LOGGER_DIR "logger")
set_or_external_dir(METAMOD_DIR "metamod-source")
set_or_external_dir(SOURCESDK_DIR "sourcesdk")

#set(SOURCESDK_MALLOC_OVERRIDE ON CACHE INTERNAL "")
set(SOURCESDK_LINK_STRIP_CPP_EXPORTS ON CACHE INTERNAL "")
set(SOURCESDK_LINK_STEAMWORKS OFF CACHE INTERNAL "")

include(cmake/logger.cmake)
include(cmake/metamod.cmake)
include(cmake/sourcesdk.cmake)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING
        "Only do Release and Debug"
        FORCE
)

set(PLUGIFY_PROJECT_VERSION "0" CACHE STRING "Set version name")

# We target plugify as object to have exported methods of plugify
set(PLUGIFY_BUILD_OBJECT_LIB ON CACHE INTERNAL "")
if(LINUX)
    set(PLUGIFY_USE_STATIC_STDLIB ON CACHE INTERNAL "")
endif()
add_subdirectory(external/plugify)

# Sources
file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${LOGGER_INCLUDE_DIRS}
        ${METAMOD_INCLUDE_DIRS}
        ${SOURCESDK_INCLUDE_DIRS}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
	C_STANDARD 11
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF

	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "plugify")

target_compile_definitions(${PROJECT_NAME} PRIVATE
        ${METAMOD_COMPILE_DEFINITIONS}

        PLUGIFY_PROJECT_YEAR="${PROJECT_BUILD_DATE_YEAR}"
        PLUGIFY_PROJECT_NAME="${PROJECT_NAME}"
        PLUGIFY_PROJECT_DESCRIPTION="${CMAKE_PROJECT_DESCRIPTION}"
        PLUGIFY_PROJECT_HOMEPAGE_URL="${CMAKE_PROJECT_HOMEPAGE_URL}"
        PLUGIFY_PROJECT_VERSION="${PLUGIFY_PROJECT_VERSION}"
)

set(PLUGIFY_LINK_LIBRARIES ${LOGGER_BINARY_DIR} plugify::plugify sourcesdk miniz)

if(WIN32)
    set(PLUGIFY_LINK_LIBRARIES ${PLUGIFY_LINK_LIBRARIES} winhttp.lib)
else()
    set(PLUGIFY_LINK_LIBRARIES ${PLUGIFY_LINK_LIBRARIES} curl)
endif()

if(NOT COMPILER_SUPPORTS_FORMAT)
    set(PLUGIFY_LINK_LIBRARIES ${PLUGIFY_LINK_LIBRARIES} fmt::fmt-header-only)
endif()

# Libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${PLUGIFY_LINK_LIBRARIES})

if(APPLE)
    target_link_options(${PROJECT_NAME} PRIVATE "-Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/sym/exported_symbols.lds")
elseif(UNIX)
    target_link_options(${PROJECT_NAME} PRIVATE "-Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/sym/version_script.lds")
endif()

if(LINUX)
    set(PLUGIFY_PREFER_OWN_SYMBOLS "true")
else()
    set(PLUGIFY_PREFER_OWN_SYMBOLS "false")
endif()

configure_file(
        ${CMAKE_SOURCE_DIR}/plugify.pconfig.in
        ${CMAKE_BINARY_DIR}/plugify.pconfig
)

if (WIN32)
    set(PLUGIFY_VDF_PLATFORM "win64")
else()
    set(PLUGIFY_VDF_PLATFORM "linuxsteamrt64")
endif()

configure_file(
        ${CMAKE_SOURCE_DIR}/plugify.vdf.in
        ${CMAKE_BINARY_DIR}/addons/metamod/plugify.vdf
)
