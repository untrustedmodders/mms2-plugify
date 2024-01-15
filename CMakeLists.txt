cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()


project(cs2wizard VERSION 1.0.0.0  DESCRIPTION "Wizard CS2 Core" HOMEPAGE_URL "https://github.com/untrustedmodders/cs2wizard" LANGUAGES C CXX ASM)

set(WIZARD_BUILD_SHARED_LIB OFF CACHE INTERNAL "")
add_subdirectory(external/wizard)

include("cmake/shared.cmake")

SET(SOURCE_FILES
    src/mm_plugin.cpp
    src/mm_plugin.h
    src/mm_logger.cpp
    src/mm_logger.h
    external/hl2sdk-cs2/tier1/convar.cpp
    #external/hl2sdk-cs2/tier1/generichash.cpp
    #external/metamod-source/core/sourcehook/sourcehook.cpp
    #external/metamod-source/core/sourcehook/sourcehook_impl_chookidman.cpp
    #external/metamod-source/core/sourcehook/sourcehook_impl_chookmaninfo.cpp
    #external/metamod-source/core/sourcehook/sourcehook_impl_cvfnptr.cpp
    #external/metamod-source/core/sourcehook/sourcehook_impl_cproto.cpp
)

if (LINUX)
    # memoverride.cpp is not usable on CMake Windows, because CMake default link libraries always link ucrt.lib
    set(SOURCE_FILES
        ${SOURCE_FILES}
        external/hl2sdk-cs2/public/tier0/memoverride.cpp
    )
endif()

# Sources
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${NATIVES_SOURCES} ${CONVERSIONS_SOURCES} ${CONVERSIONS_HEADERS})

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cs2_sdk
)

if(LINUX)
    include("cmake/linux.cmake")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/addons/wizard/bin/linuxsteamrt64"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/addons/wizard/bin/linuxsteamrt64"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/addons/wizard/bin/linuxsteamrt64"
    )
elseif(WIN32)
    include("cmake/windows.cmake")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        PREFIX ""
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/addons/wizard/bin/win64"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/addons/wizard/bin/win64"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/addons/wizard/bin/win64"
    )
endif()

# Libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${WIZARD_LINK_LIBRARIES})