# find the folder into which the OpenXR loader has been installed

# early out, if this target has been created before
if(TARGET nsOpenXR::Loader)
	return()
endif()

set(NS_OPENXR_LOADER_DIR "NS_OPENXR_LOADER_DIR-NOTFOUND" CACHE PATH "Directory of OpenXR loader installation")
set(NS_OPENXR_HEADERS_DIR "NS_OPENXR_HEADERS_DIR-NOTFOUND" CACHE PATH "Directory of OpenXR headers installation")
set(NS_OPENXR_PREVIEW_DIR "" CACHE PATH "Directory of OpenXR preview include root")
set(NS_OPENXR_REMOTING_DIR "" CACHE PATH "Directory of OpenXR remoting installation")
mark_as_advanced(FORCE NS_OPENXR_LOADER_DIR)
mark_as_advanced(FORCE NS_OPENXR_HEADERS_DIR)
mark_as_advanced(FORCE NS_OPENXR_PREVIEW_DIR)
mark_as_advanced(FORCE NS_OPENXR_REMOTING_DIR)

ns_pull_compiler_and_architecture_vars()

if((NS_OPENXR_LOADER_DIR STREQUAL "NS_OPENXR_LOADER_DIR-NOTFOUND") OR(NS_OPENXR_LOADER_DIR STREQUAL "") OR(NS_OPENXR_HEADERS_DIR STREQUAL "NS_OPENXR_HEADERS_DIR-NOTFOUND") OR(NS_OPENXR_HEADERS_DIR STREQUAL "") OR(NS_OPENXR_REMOTING_DIR STREQUAL "NS_OPENXR_REMOTING_DIR-NOTFOUND") OR(NS_OPENXR_REMOTING_DIR STREQUAL ""))
	ns_nuget_init()
	execute_process(COMMAND ${NUGET} restore ${CMAKE_SOURCE_DIR}/Code/EnginePlugins/OpenXRPlugin/packages.config -PackagesDirectory ${CMAKE_BINARY_DIR}/packages
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
	set(NS_OPENXR_LOADER_DIR "${CMAKE_BINARY_DIR}/packages/OpenXR.Loader.1.0.10.2" CACHE PATH "Directory of OpenXR loader installation" FORCE)
	set(NS_OPENXR_HEADERS_DIR "${CMAKE_BINARY_DIR}/packages/OpenXR.Headers.1.0.10.2" CACHE PATH "Directory of OpenXR headers installation" FORCE)
	set(NS_OPENXR_REMOTING_DIR "${CMAKE_BINARY_DIR}/packages/Microsoft.Holographic.Remoting.OpenXr.2.4.0" CACHE PATH "Directory of OpenXR remoting installation" FORCE)
endif()

if(NS_CMAKE_PLATFORM_WINDOWS_UWP)
	set(OPENXR_DYNAMIC ON)
	find_path(NS_OPENXR_HEADERS_DIR include/openxr/openxr.h)

	if(NS_CMAKE_ARCHITECTURE_ARM)
		if(NS_CMAKE_ARCHITECTURE_64BIT)
			set(OPENXR_BIN_PREFIX "arm64_uwp")
		else()
			set(OPENXR_BIN_PREFIX "arm_uwp")
		endif()
	else()
		if(NS_CMAKE_ARCHITECTURE_64BIT)
			set(OPENXR_BIN_PREFIX "x64_uwp")
		else()
			set(OPENXR_BIN_PREFIX "Win32_uwp")
		endif()
	endif()

elseif(NS_CMAKE_PLATFORM_WINDOWS_DESKTOP)
	set(OPENXR_DYNAMIC ON)
	find_path(NS_OPENXR_HEADERS_DIR include/openxr/openxr.h)

	if(NS_CMAKE_ARCHITECTURE_64BIT)
		set(OPENXR_BIN_PREFIX "x64")
		find_path(NS_OPENXR_REMOTING_DIR build/native/include/openxr/openxr_msft_holographic_remoting.h)
	else()
		set(OPENXR_BIN_PREFIX "Win32")
	endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(nsOpenXR DEFAULT_MSG NS_OPENXR_LOADER_DIR)
find_package_handle_standard_args(nsOpenXR DEFAULT_MSG NS_OPENXR_HEADERS_DIR)
find_package_handle_standard_args(nsOpenXR DEFAULT_MSG NS_OPENXR_REMOTING_DIR)

if(NSOPENXR_FOUND)
	add_library(nsOpenXR::Loader SHARED IMPORTED)

	if(OPENXR_DYNAMIC)
		set_target_properties(nsOpenXR::Loader PROPERTIES IMPORTED_LOCATION "${NS_OPENXR_LOADER_DIR}/native/${OPENXR_BIN_PREFIX}/release/bin/openxr_loader.dll")
		set_target_properties(nsOpenXR::Loader PROPERTIES IMPORTED_LOCATION_DEBUG "${NS_OPENXR_LOADER_DIR}/native/${OPENXR_BIN_PREFIX}/release/bin/openxr_loader.dll")
	endif()

	set_target_properties(nsOpenXR::Loader PROPERTIES IMPORTED_IMPLIB "${NS_OPENXR_LOADER_DIR}/native/${OPENXR_BIN_PREFIX}/release/lib/openxr_loader.lib")
	set_target_properties(nsOpenXR::Loader PROPERTIES IMPORTED_IMPLIB_DEBUG "${NS_OPENXR_LOADER_DIR}/native/${OPENXR_BIN_PREFIX}/release/lib/openxr_loader.lib")

	set_target_properties(nsOpenXR::Loader PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${NS_OPENXR_HEADERS_DIR}/include")

	if(NOT NS_OPENXR_PREVIEW_DIR STREQUAL "")
		set_target_properties(nsOpenXR::Loader PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${NS_OPENXR_HEADERS_DIR}/include")
	endif()

	ns_uwp_mark_import_as_content(nsOpenXR::Loader)

	if(NS_CMAKE_PLATFORM_WINDOWS_DESKTOP AND NS_CMAKE_ARCHITECTURE_64BIT)

		add_library(nsOpenXR::Remoting INTERFACE IMPORTED)

		if(NS_CMAKE_PLATFORM_WINDOWS_UWP)
			list(APPEND REMOTING_ASSETS "${NS_OPENXR_REMOTING_DIR}/build/native/bin/x64/uwp/RemotingXR.json")
			list(APPEND REMOTING_ASSETS "${NS_OPENXR_REMOTING_DIR}/build/native/bin/x64/uwp/Microsoft.Holographic.AppRemoting.OpenXr.dll")
		else()
			list(APPEND REMOTING_ASSETS "${NS_OPENXR_REMOTING_DIR}/build/native/bin/x64/Desktop/RemotingXR.json")
			list(APPEND REMOTING_ASSETS "${NS_OPENXR_REMOTING_DIR}/build/native/bin/x64/Desktop/Microsoft.Holographic.AppRemoting.OpenXr.dll")
		endif()

		set_target_properties(nsOpenXR::Remoting PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${NS_OPENXR_REMOTING_DIR}/build/native/include")
		set_target_properties(nsOpenXR::Remoting PROPERTIES INTERFACE_SOURCES "${REMOTING_ASSETS}")

		set_property(SOURCE ${REMOTING_ASSETS} PROPERTY VS_DEPLOYMENT_CONTENT 1)
		set_property(SOURCE ${REMOTING_ASSETS} PROPERTY VS_DEPLOYMENT_LOCATION "")

	endif()

	
endif()

unset(OPENXR_DYNAMIC)
unset(OPENXR_BIN_PREFIX)
