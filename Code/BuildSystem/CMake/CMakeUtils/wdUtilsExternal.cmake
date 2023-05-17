# #####################################
# ## wd_include_ezExport()
# #####################################

macro(wd_include_ezExport)
	# Create a modified version of the ezExport.cmake file,
	# where the absolute paths to the original locations are replaced
	# with the absolute paths to this installation
	set(EXP_FILE "${WD_OUTPUT_DIRECTORY_DLL}/ezExport.cmake")
	set(IMP_FILE "${CMAKE_BINARY_DIR}/ezExport.cmake")
	set(EXPINFO_FILE "${WD_OUTPUT_DIRECTORY_DLL}/ezExportInfo.cmake")

	# read the file that contains the original paths
	include(${EXPINFO_FILE})

	# read the ezExport file into a string
	file(READ ${EXP_FILE} IMP_CONTENT)

	# replace the original paths with our paths
	string(REPLACE ${EXPINP_OUTPUT_DIRECTORY_DLL} ${WD_OUTPUT_DIRECTORY_DLL} IMP_CONTENT "${IMP_CONTENT}")
	string(REPLACE ${EXPINP_OUTPUT_DIRECTORY_LIB} ${WD_OUTPUT_DIRECTORY_LIB} IMP_CONTENT "${IMP_CONTENT}")
	string(REPLACE ${EXPINP_SOURCE_DIR} ${WD_SDK_DIR} IMP_CONTENT "${IMP_CONTENT}")

	# write the modified ezExport file to disk
	file(WRITE ${IMP_FILE} "${IMP_CONTENT}")

	# include the modified file, so that the CMake targets become known
	include(${IMP_FILE})
endmacro()

# #####################################
# ## wd_configure_external_project()
# #####################################
macro(wd_configure_external_project)

	if (WD_SDK_DIR STREQUAL "")
		file(RELATIVE_PATH WD_SUBMODULE_PREFIX_PATH ${CMAKE_SOURCE_DIR} ${WD_SDK_DIR})
	else()
		set(WD_SUBMODULE_PREFIX_PATH "")
	endif()
	
	set_property(GLOBAL PROPERTY WD_SUBMODULE_PREFIX_PATH ${WD_SUBMODULE_PREFIX_PATH})

	if(WD_SUBMODULE_PREFIX_PATH STREQUAL "")
		set(WD_SUBMODULE_MODE FALSE)
	else()
		set(WD_SUBMODULE_MODE TRUE)
	endif()

	set_property(GLOBAL PROPERTY WD_SUBMODULE_MODE ${WD_SUBMODULE_MODE})

	wd_build_filter_init()

	wd_set_build_types()
endmacro()