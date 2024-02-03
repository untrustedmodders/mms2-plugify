if (WIN32)
	set(PLUGIFY_VDF_PLATFORM "win64")
else()
	set(PLUGIFY_VDF_PLATFORM "linuxsteamrt64")
endif()

configure_file(
	${CMAKE_CURRENT_LIST_DIR}/plugify.vdf.in
	${PROJECT_SOURCE_DIR}/configs/addons/metamod/plugify.vdf
)