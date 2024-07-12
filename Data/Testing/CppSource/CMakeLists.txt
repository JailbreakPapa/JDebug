cmake_minimum_required(VERSION 3.21)

project (AniDriveProto VERSION 1.0 LANGUAGES C CXX)

if(PROJECT_IS_TOP_LEVEL)

	set (NS_SDK_DIR "" CACHE PATH "The nsEngine SDK root directory.")

	message(STATUS "NS_SDK_DIR is set to '${NS_SDK_DIR}'")

	# fix output directory
	set (NS_OUTPUT_DIRECTORY_LIB "${NS_SDK_DIR}/Output/Lib" CACHE PATH "Where to store the compiled .lib files." FORCE)
	set (NS_OUTPUT_DIRECTORY_DLL "${NS_SDK_DIR}/Output/Bin" CACHE PATH "Where to store the compiled .dll files." FORCE)

	include("${NS_SDK_DIR}/nsCMakeConfig.cmake")
	get_property(NS_CMAKE_RELPATH GLOBAL PROPERTY NS_CMAKE_RELPATH)
	get_property(NS_CMAKE_RELPATH_CODE GLOBAL PROPERTY NS_CMAKE_RELPATH_CODE)

	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${NS_SDK_DIR}/${NS_CMAKE_RELPATH}")
	
	set(NS_ROOT ${NS_SDK_DIR})
	set_property(GLOBAL PROPERTY NS_ROOT ${NS_ROOT})

	include("nsUtils")
	include("nsPlatformVars")

	ns_include_nsExport()

	ns_configure_external_project()

endif()

# include all projects in the sub-folders
ns_add_all_subdirs()
