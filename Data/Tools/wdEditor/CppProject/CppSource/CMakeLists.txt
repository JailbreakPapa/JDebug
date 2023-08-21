cmake_minimum_required(VERSION 3.21)

project (CppProject VERSION 1.0 LANGUAGES C CXX)

if(PROJECT_IS_TOP_LEVEL)

	set (WD_SDK_DIR "" CACHE PATH "The wdEngine SDK root directory.")

	message(STATUS "WD_SDK_DIR is set to '${WD_SDK_DIR}'")

	# fix output directory
	set (WD_OUTPUT_DIRECTORY_LIB "${WD_SDK_DIR}/Output/Lib" CACHE PATH "Where to store the compiled .lib files." FORCE)
	set (WD_OUTPUT_DIRECTORY_DLL "${WD_SDK_DIR}/Output/Bin" CACHE PATH "Where to store the compiled .dll files." FORCE)

	include("${WD_SDK_DIR}/wdCMakeConfig.cmake")
	get_property(WD_CMAKE_RELPATH GLOBAL PROPERTY WD_CMAKE_RELPATH)
	get_property(WD_CMAKE_RELPATH_CODE GLOBAL PROPERTY WD_CMAKE_RELPATH_CODE)

	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${WD_SDK_DIR}/${WD_CMAKE_RELPATH}")

	include("wdUtils")
	include("wdPlatformVars")

	wd_include_wdExport()

	wd_configure_external_project()

endif()

# include all projects in the sub-folders
wd_add_all_subdirs()
