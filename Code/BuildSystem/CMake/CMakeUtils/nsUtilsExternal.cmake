# #####################################
# ## ns_include_nsExport()
# #####################################

macro(ns_include_nsExport)
	# Create a modified version of the nsExport.cmake file,
	# where the absolute paths to the original locations are replaced
	# with the absolute paths to this installation
	ns_get_export_location(EXP_FILE)
	set(IMP_FILE "${CMAKE_BINARY_DIR}/nsExport.cmake")
	set(EXPINFO_FILE "${NS_OUTPUT_DIRECTORY_DLL}/nsExportInfo.cmake")

	# read the file that contains the original paths
	include(${EXPINFO_FILE})

	# read the nsExport file into a string
	file(READ ${EXP_FILE} IMP_CONTENT)

	# replace the original paths with our paths
	string(REPLACE ${EXPINP_OUTPUT_DIRECTORY_DLL} ${NS_OUTPUT_DIRECTORY_DLL} IMP_CONTENT "${IMP_CONTENT}")
	string(REPLACE ${EXPINP_OUTPUT_DIRECTORY_LIB} ${NS_OUTPUT_DIRECTORY_LIB} IMP_CONTENT "${IMP_CONTENT}")
	string(REPLACE ${EXPINP_SOURCE_DIR} ${NS_SDK_DIR} IMP_CONTENT "${IMP_CONTENT}")

	# write the modified nsExport file to disk
	file(WRITE ${IMP_FILE} "${IMP_CONTENT}")

	# include the modified file, so that the CMake targets become known
	include(${IMP_FILE})
endmacro()

# #####################################
# ## ns_configure_external_project()
# #####################################
macro(ns_configure_external_project)

	if (NS_SDK_DIR STREQUAL "")
		file(RELATIVE_PATH NS_SUBMODULE_PREFIX_PATH ${CMAKE_SOURCE_DIR} ${NS_SDK_DIR})
	else()
		set(NS_SUBMODULE_PREFIX_PATH "")
	endif()
	
	set_property(GLOBAL PROPERTY NS_SUBMODULE_PREFIX_PATH ${NS_SUBMODULE_PREFIX_PATH})

	if(NS_SUBMODULE_PREFIX_PATH STREQUAL "")
		set(NS_SUBMODULE_MODE FALSE)
	else()
		set(NS_SUBMODULE_MODE TRUE)
	endif()

	set_property(GLOBAL PROPERTY NS_SUBMODULE_MODE ${NS_SUBMODULE_MODE})

	ns_build_filter_init()

	ns_set_build_types()
endmacro()