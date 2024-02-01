cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()


project(cs2-wizard VERSION 1.0.0.0  DESCRIPTION "Wizard CS2 Core" HOMEPAGE_URL "https://github.com/untrustedmodders/cs2wizard" LANGUAGES C CXX ASM)

# We target wizard as object to have exported methods of wizard
set(WIZARD_BUILD_OBJECT_LIB ON CACHE INTERNAL "")
add_subdirectory(external/wizard)

include("cmake/shared.cmake")

SET(SOURCE_FILES
    src/mm_plugin.cpp
    src/mm_plugin.h
    src/mm_logger.cpp
    src/mm_logger.h
    external/hl2sdk-cs2/tier1/convar.cpp
)

if (LINUX)
    # memoverride.cpp is not usable on CMake Windows, because CMake default link libraries always link ucrt.lib
    set(SOURCE_FILES
        ${SOURCE_FILES}
        external/hl2sdk-cs2/public/tier0/memoverride.cpp
    )
endif()

## Generate protobuf source & headers
if (LINUX)
	set(PROTOC_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/external/hl2sdk-cs2/devtools/bin/linux/protoc)
elseif(WIN32)
	set(PROTOC_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/external/hl2sdk-cs2/devtools/bin/protoc.exe)
endif()

add_custom_command(
		OUTPUT protobuf_output_stamp
		COMMAND ${PROTOC_EXECUTABLE} --proto_path=thirdparty/protobuf-3.21.8/src --proto_path=common --cpp_out=common common/network_connection.proto
		COMMENT "Generating protobuf file"
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external/hl2sdk-cs2
		VERBATIM
)

SET(SOURCE_FILES ${SOURCE_FILES} protobuf_output_stamp)

# Sources
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${NATIVES_SOURCES} ${CONVERSIONS_SOURCES} ${CONVERSIONS_HEADERS})

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/cs2_sdk
        ${CMAKE_BINARY_DIR}/exports
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

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "wizard")

target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIZARD_PROJECT_NAME="${PROJECT_NAME}"
        WIZARD_PROJECT_DESCRIPTION="${CMAKE_PROJECT_DESCRIPTION}"
        WIZARD_PROJECT_HOMEPAGE_URL="${CMAKE_PROJECT_HOMEPAGE_URL}"
        WIZARD_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

# Libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${WIZARD_LINK_LIBRARIES})