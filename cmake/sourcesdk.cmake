# mms2-plugify
# Copyright (C) 2024 untrustedmodders
# Licensed under the MIT license. See LICENSE file in the project root for details.

if(NOT SOURCESDK_DIR)
	message(FATAL_ERROR "SOURCESDK_DIR is empty")
endif()

set(SOURCESDK_BINARY_DIR "sourcesdk")

set(SOURCESDK_LINK_STRIP_CPP_EXPORTS ON CACHE INTERNAL "")
set(SOURCESDK_LINK_STEAMWORKS ON CACHE INTERNAL "")
set(SOURCESDK_CONFIGURE_EXPORT_MAP OFF CACHE INTERNAL "")

add_subdirectory(${SOURCESDK_DIR} ${SOURCESDK_BINARY_DIR})

function(get_sourcesdk_target_property VAR_NAME TARGET PROPERTY)
	get_target_property(PROPERTY_VALUE ${TARGET} ${PROPERTY})

	if("${PROPERTY_VALUE}" MATCHES "PROPERTY_VALUE-NOTFOUND")
		set(${VAR_NAME} PARENT_SCOPE)
	else()
		set(${VAR_NAME} ${PROPERTY_VALUE} PARENT_SCOPE)
	endif()
endfunction()
