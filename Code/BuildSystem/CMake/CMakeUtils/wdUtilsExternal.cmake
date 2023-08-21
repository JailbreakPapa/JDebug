# #####################################
# ## wd_include_wdExport()
# #####################################

macro(wd_include_wdExport)
	# Create a modified version of the wdExport.cmake file,
	# where the absolute paths to the original locations are replaced
	# with the absolute paths to this installation
	set(EXP_FILE "${WD_OUTPUT_DIRECTORY_DLL}/wdExport.cmake")
	set(IMP_FILE "${CMAKE_BINARY_DIR}/wdExport.cmake")
	set(EXPINFO_FILE "${WD_OUTPUT_DIRECTORY_DLL}/wdExportInfo.cmake")

	# read the file that contains the original paths
	include(${EXPINFO_FILE})

	# read the wdExport file into a string
	file(READ ${EXP_FILE} IMP_CONTENT)

	# replace the original paths with our paths
	string(REPLACE ${EXPINP_OUTPUT_DIRECTORY_DLL} ${WD_OUTPUT_DIRECTORY_DLL} IMP_CONTENT "${IMP_CONTENT}")
	string(REPLACE ${EXPINP_OUTPUT_DIRECTORY_LIB} ${WD_OUTPUT_DIRECTORY_LIB} IMP_CONTENT "${IMP_CONTENT}")
	string(REPLACE ${EXPINP_SOURCE_DIR} ${WD_SDK_DIR} IMP_CONTENT "${IMP_CONTENT}")

	# write the modified wdExport file to disk
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