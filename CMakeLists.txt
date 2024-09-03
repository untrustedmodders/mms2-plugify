cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()


project(cs2-plugify VERSION 1.0.0.0 DESCRIPTION "Plugify CS2 Core" HOMEPAGE_URL "https://github.com/untrustedmodders/cs2-plugify" LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if(WIN32 AND NOT MSVC)
    message(FATAL "MSVC restricted.")
endif()

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

add_subdirectory(external/sourcesdk)

# Sources
file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

set(SOURCESDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/sourcesdk)
set(METAMOD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/metamod-source)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${SOURCESDK_DIR}
        ${SOURCESDK_DIR}/common
        ${SOURCESDK_DIR}/game/shared
        ${SOURCESDK_DIR}/game/server
        ${SOURCESDK_DIR}/public
        ${SOURCESDK_DIR}/public/engine
        ${SOURCESDK_DIR}/public/mathlib
        ${SOURCESDK_DIR}/public/tier0
        ${SOURCESDK_DIR}/public/tier1
        ${SOURCESDK_DIR}/public/entity2
        ${SOURCESDK_DIR}/public/game/server
        ${SOURCESDK_DIR}/public/entity2

        ${METAMOD_DIR}/core
        ${METAMOD_DIR}/core/sourcehook
)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "plugify")

target_compile_definitions(${PROJECT_NAME} PRIVATE
        PLUGIFY_PROJECT_NAME="${PROJECT_NAME}"
        PLUGIFY_PROJECT_DESCRIPTION="${CMAKE_PROJECT_DESCRIPTION}"
        PLUGIFY_PROJECT_HOMEPAGE_URL="${CMAKE_PROJECT_HOMEPAGE_URL}"
        PLUGIFY_PROJECT_VERSION="${PLUGIFY_PROJECT_VERSION}")

if(LINUX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
            META_IS_SOURCE2
            _LINUX
            POSIX
            LINUX
            GNUC
            COMPILER_GCC
            PLATFORM_64BITS
            _GLIBCXX_USE_CXX11_ABI=0
    )
elseif(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
            META_IS_SOURCE2
            COMPILER_MSVC
            COMPILER_MSVC64
            _WIN32
            _WINDOWS
            _ALLOW_KEYWORD_MACROS
            __STDC_LIMIT_MACROS
            _CRT_SECURE_NO_WARNINGS=1
            _CRT_SECURE_NO_DEPRECATE=1
            _CRT_NONSTDC_NO_DEPRECATE=1
    )
    #set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(PLUGIFY_LINK_LIBRARIES plugify::plugify sourcesdk miniz)

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
