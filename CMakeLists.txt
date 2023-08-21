cmake_minimum_required(VERSION 3.20)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "In-source builds are not supported. Please choose a different binary directory.")
	return()
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/wdCMakeConfig.cmake")
get_property(WD_CMAKE_RELPATH GLOBAL PROPERTY WD_CMAKE_RELPATH)
get_property(WD_CMAKE_RELPATH_CODE GLOBAL PROPERTY WD_CMAKE_RELPATH_CODE)

# Required to find cmake modules like 'FindDirectX.cmake'
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/${WD_CMAKE_RELPATH}")

include("wdUtils")

file(RELATIVE_PATH WD_SUBMODULE_PREFIX_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
set_property(GLOBAL PROPERTY WD_SUBMODULE_PREFIX_PATH ${WD_SUBMODULE_PREFIX_PATH})

if(WD_SUBMODULE_PREFIX_PATH STREQUAL "")
	set(WD_SUBMODULE_MODE FALSE)
else()
	set(WD_SUBMODULE_MODE TRUE)
endif()

set_property(GLOBAL PROPERTY WD_SUBMODULE_MODE ${WD_SUBMODULE_MODE})

wd_build_filter_init()

wd_detect_project_name(DETECTED_NAME)

if(NOT WD_SUBMODULE_MODE)
	wd_detect_languages()

	set(WD_SOLUTION_NAME ${DETECTED_NAME} CACHE STRING "The name of the solution")
	project(${WD_SOLUTION_NAME}
		VERSION 1.0
		DESCRIPTION "Watch Dogs LLC In-House Engine"
		LANGUAGES ${WD_LANGUAGES})
endif()

wd_set_build_types()
wd_check_build_type()
wd_write_configuration_txt()

if(WD_CUSTOM_TOOLCHAIN_FILE)
	message(STATUS "Including WD_CUSTOM_TOOLCHAIN_FILE: '${WD_CUSTOM_TOOLCHAIN_FILE}'")
	include("${WD_CUSTOM_TOOLCHAIN_FILE}")
endif()

wd_pull_all_vars()

include("wdPlatformVars")

set(WD_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Code")

wd_find_qt()
wd_init_projects()

get_property(SELECTED_FILTER_NAME GLOBAL PROPERTY WD_BUILD_FILTER_SELECTED)
if (NOT ${SELECTED_FILTER_NAME} MATCHES "Everything")
	set(CMAKE_SUPPRESS_REGENERATION true)
endif()
add_subdirectory(${WD_CMAKE_RELPATH_CODE})

wd_add_external_projects_folder("_1")
wd_add_external_projects_folder("_2")
wd_add_external_projects_folder("_3")

if(NOT WD_SUBMODULE_MODE)
	if(TARGET Editor)
		# sets the Editor project as the default startup project in Visual Studio
		set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT Editor)
	endif()
endif()

# finalizers may run into problems in split solutions
# however, they aren't needed there, since CI will always build all projects
# and finalizers are typically only needed to set up build dependencies
if (${SELECTED_FILTER_NAME} MATCHES "Everything")
wd_finalize_projects()
endif()

get_property(EXPORT_PROJECTS GLOBAL PROPERTY "EXPORT_PROJECTS")
export(TARGETS ${EXPORT_PROJECTS} FILE "${WD_OUTPUT_DIRECTORY_DLL}/wdExport.cmake")

set(EXPORT_INFO "
set(EXPINP_OUTPUT_DIRECTORY_DLL ${WD_OUTPUT_DIRECTORY_DLL})
set(EXPINP_OUTPUT_DIRECTORY_LIB ${WD_OUTPUT_DIRECTORY_LIB})
set(EXPINP_BINARY_DIR ${CMAKE_BINARY_DIR})
set(EXPINP_SOURCE_DIR ${CMAKE_SOURCE_DIR})
")

file(WRITE "${WD_OUTPUT_DIRECTORY_DLL}/wdExportInfo.cmake" ${EXPORT_INFO})
