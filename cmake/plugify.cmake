# mms2-plugify
# Copyright (C) 2024 untrustedmodders
# Licensed under the MIT license. See LICENSE file in the project root for details.

if(NOT PLUGIFY_DIR)
	message(FATAL_ERROR "PLUGIFY_DIR is empty")
endif()

set(PLUGIFY_BINARY_DIR "plugify")

# We target plugify as object to have exported methods of plugify
set(PLUGIFY_BUILD_OBJECT_LIB ON CACHE INTERNAL "")
set(PLUGIFY_BUILD_TESTS OFF CACHE INTERNAL "")
if(LINUX)
	set(PLUGIFY_USE_STATIC_STDLIB ON CACHE INTERNAL "")
endif()

add_subdirectory(${PLUGIFY_DIR} ${PLUGIFY_BINARY_DIR})

set(PLUGIFY_COMPILE_DEFINITIONS
		#${PLUGIFY_COMPILE_DEFINITIONS}

		PLUGIFY_LIBRARY_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}"
		PLUGIFY_LIBRARY_PREFIX="${CMAKE_SHARED_LIBRARY_PREFIX}"
		PLUGIFY_PROJECT_YEAR="${PROJECT_BUILD_DATE_YEAR}"
		PLUGIFY_PROJECT_NAME="${PROJECT_NAME}"
		PLUGIFY_PROJECT_DESCRIPTION="${CMAKE_PROJECT_DESCRIPTION}"
		PLUGIFY_PROJECT_HOMEPAGE_URL="${CMAKE_PROJECT_HOMEPAGE_URL}"
		PLUGIFY_PROJECT_VERSION="${PLUGIFY_PROJECT_VERSION}"
)

if(WINDOWS)
	set(PLUGIFY_COMPILE_DEFINITIONS
			${PLUGIFY_COMPILE_DEFINITIONS}
			PLUGIFY_PLATFORM="windows"
			PLUGIFY_BINARY="win64"
			PLUGIFY_ROOT_BINARY="/bin/win64/"
			PLUGIFY_GAME_BINARY="/csgo/bin/win64/"
	)
elseif(LINUX)
	set(PLUGIFY_COMPILE_DEFINITIONS
			${PLUGIFY_COMPILE_DEFINITIONS}
			PLUGIFY_PLATFORM="linux"
			PLUGIFY_BINARY="linuxsteamrt64"
			PLUGIFY_ROOT_BINARY="/bin/linuxsteamrt64/"
			PLUGIFY_GAME_BINARY="/bin/linuxsteamrt64/../../csgo/bin/linuxsteamrt64/" # Metamod hack
	)
endif()

set(PLUGIFY_LINK_LIBRARIES ${LOGGER_BINARY_DIR} miniz)

if(WIN32)
	list(APPEND PLUGIFY_LINK_LIBRARIES winhttp.lib)
else()
	list(APPEND PLUGIFY_LINK_LIBRARIES curl)
endif()

if(NOT COMPILER_SUPPORTS_FORMAT)
	list(APPEND PLUGIFY_LINK_LIBRARIES fmt::fmt-header-only)
endif()
