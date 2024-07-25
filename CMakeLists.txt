cmake_minimum_required(VERSION 3.20)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "In-source builds are not supported. Please choose a different binary directory.")
	return()
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/nsCMakeConfig.cmake")
get_property(NS_CMAKE_RELPATH GLOBAL PROPERTY NS_CMAKE_RELPATH)
get_property(NS_CMAKE_RELPATH_CODE GLOBAL PROPERTY NS_CMAKE_RELPATH_CODE)

# Required to find cmake modules like 'FindDirectX.cmake'
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/${NS_CMAKE_RELPATH}")

include("nsUtils")

file(RELATIVE_PATH NS_SUBMODULE_PREFIX_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
set_property(GLOBAL PROPERTY NS_SUBMODULE_PREFIX_PATH ${NS_SUBMODULE_PREFIX_PATH})

if(NS_SUBMODULE_PREFIX_PATH STREQUAL "")
	set(NS_SUBMODULE_MODE FALSE)
else()
	set(NS_SUBMODULE_MODE TRUE)
endif()

set_property(GLOBAL PROPERTY NS_SUBMODULE_MODE ${NS_SUBMODULE_MODE})

# Set a global variable pointing to the root of the NS source tree
set(NS_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set_property(GLOBAL PROPERTY NS_ROOT ${NS_ROOT})

ns_build_filter_init()

ns_detect_project_name(DETECTED_NAME)

if(NOT NS_SUBMODULE_MODE)
	ns_detect_languages()

	set(NS_SOLUTION_NAME ${DETECTED_NAME} CACHE STRING "The name of the solution")
	project(${NS_SOLUTION_NAME}
		VERSION 1.0
		DESCRIPTION "An open-source game engine"
		HOMEPAGE_URL "https://nsengine.net"
		LANGUAGES ${NS_LANGUAGES})
endif()

include("nsPlatform")

ns_set_build_types()
ns_check_build_type()
ns_write_configuration_txt()

if(NS_CUSTOM_TOOLCHAIN_FILE)
	message(STATUS "Including NS_CUSTOM_TOOLCHAIN_FILE: '${NS_CUSTOM_TOOLCHAIN_FILE}'")
	include("${NS_CUSTOM_TOOLCHAIN_FILE}")
endif()

ns_pull_all_vars()

set(NS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Code")

ns_find_qt()
ns_init_projects()

get_property(SELECTED_FILTER_NAME GLOBAL PROPERTY NS_BUILD_FILTER_SELECTED)
if (NOT ${SELECTED_FILTER_NAME} MATCHES "Everything")
	set(CMAKE_SUPPRESS_REGENERATION true)
endif()
add_subdirectory(${NS_CMAKE_RELPATH_CODE})
add_subdirectory(Data)

ns_add_external_projects_folder("_1")
ns_add_external_projects_folder("_2")
ns_add_external_projects_folder("_3")

if(NOT NS_SUBMODULE_MODE)
	if(TARGET Editor)
		# sets the Editor project as the default startup project in Visual Studio
		set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT Editor)
	endif()
endif()

# finalizers may run into problems in split solutions
# however, they aren't needed there, since CI will always build all projects
# and finalizers are typically only needed to set up build dependencies
if (${SELECTED_FILTER_NAME} MATCHES "Everything")
ns_finalize_projects()
endif()

get_property(EXPORT_PROJECTS GLOBAL PROPERTY "EXPORT_PROJECTS")
ns_get_export_location(NS_EXPORT_LOCATION)
export(TARGETS ${EXPORT_PROJECTS} FILE "${NS_EXPORT_LOCATION}")

set(EXPORT_INFO "
set(EXPINP_OUTPUT_DIRECTORY_DLL ${NS_OUTPUT_DIRECTORY_DLL})
set(EXPINP_OUTPUT_DIRECTORY_LIB ${NS_OUTPUT_DIRECTORY_LIB})
set(EXPINP_BINARY_DIR ${CMAKE_BINARY_DIR})
set(EXPINP_SOURCE_DIR ${CMAKE_SOURCE_DIR})
")

file(WRITE "${NS_OUTPUT_DIRECTORY_DLL}/nsExportInfo.cmake" ${EXPORT_INFO})
