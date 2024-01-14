if (WIN32)
	set(WIZARD_VDF_PLATFORM "win64")
else()
	set(WIZARD_VDF_PLATFORM "linuxsteamrt64")
endif()

configure_file(
	${CMAKE_CURRENT_LIST_DIR}/wizard.vdf.in
	${PROJECT_SOURCE_DIR}/configs/addons/metamod/wizard.vdf
)