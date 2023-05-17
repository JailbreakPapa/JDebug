# find the folder into which the Vulkan SDK has been installed

# early out, if this target has been created before
if((TARGET WdVulkan::Loader) AND(TARGET WdVulkan::DXC))
	return()
endif()

set(WD_VULKAN_DIR $ENV{VULKAN_SDK} CACHE PATH "Directory of the Vulkan SDK")

wd_pull_compiler_and_architecture_vars()
wd_pull_config_vars()

get_property(WD_SUBMODULE_PREFIX_PATH GLOBAL PROPERTY WD_SUBMODULE_PREFIX_PATH)

if(WD_CMAKE_PLATFORM_WINDOWS_DESKTOP AND WD_CMAKE_ARCHITECTURE_64BIT)
	if((WD_VULKAN_DIR STREQUAL "WD_VULKAN_DIR-NOTFOUND") OR(WD_VULKAN_DIR STREQUAL ""))
		# set(CMAKE_FIND_DEBUG_MODE TRUE)
		unset(WD_VULKAN_DIR CACHE)
		unset(WdVulkan_DIR CACHE)
		find_path(WD_VULKAN_DIR Config/vk_layer_settings.txt
			PATHS
			${WD_VULKAN_DIR}
			$ENV{VULKAN_SDK}
			REQUIRED
		)

		# set(CMAKE_FIND_DEBUG_MODE FALSE)
	endif()
elseif(WD_CMAKE_PLATFORM_LINUX AND WD_CMAKE_ARCHITECTURE_64BIT)
	if((WD_VULKAN_DIR STREQUAL "WD_VULKAN_DIR-NOTFOUND") OR(WD_VULKAN_DIR STREQUAL ""))
		# set(CMAKE_FIND_DEBUG_MODE TRUE)
		unset(WD_VULKAN_DIR CACHE)
		unset(WdVulkan_DIR CACHE)
		find_path(WD_VULKAN_DIR config/vk_layer_settings.txt
			PATHS
			${WD_VULKAN_DIR}
			$ENV{VULKAN_SDK}
		)

		if(WD_CMAKE_ARCHITECTURE_X86)
			if((WD_VULKAN_DIR STREQUAL "WD_VULKAN_DIR-NOTFOUND") OR (WD_VULKAN_DIR STREQUAL ""))
				wd_download_and_extract("${WD_CONFIG_VULKAN_SDK_LINUXX64_URL}" "${CMAKE_BINARY_DIR}/vulkan-sdk" "vulkan-sdk-${WD_CONFIG_VULKAN_SDK_LINUXX64_VERSION}")
				set(WD_VULKAN_DIR "${CMAKE_BINARY_DIR}/vulkan-sdk/${WD_CONFIG_VULKAN_SDK_LINUXX64_VERSION}" CACHE PATH "Directory of the Vulkan SDK" FORCE)

				find_path(WD_VULKAN_DIR config/vk_layer_settings.txt
					PATHS
					${WD_VULKAN_DIR}
					$ENV{VULKAN_SDK}
				)
			endif()
		endif()

		if((WD_VULKAN_DIR STREQUAL "WD_VULKAN_DIR-NOTFOUND") OR (WD_VULKAN_DIR STREQUAL ""))
			message(FATAL_ERROR "Failed to find vulkan SDK. Wd requires the vulkan sdk ${WD_CONFIG_VULKAN_SDK_LINUXX64_VERSION}. Please set the environment variable VULKAN_SDK to the vulkan sdk location.")
		endif()

		# set(CMAKE_FIND_DEBUG_MODE FALSE)
	endif()
else()
	message(FATAL_ERROR "TODO: Vulkan is not yet supported on this platform and/or architecture.")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WdVulkan DEFAULT_MSG WD_VULKAN_DIR)

if(WDVULKAN_FOUND)
	if(WD_CMAKE_PLATFORM_WINDOWS_DESKTOP AND WD_CMAKE_ARCHITECTURE_64BIT)
		add_library(WdVulkan::Loader STATIC IMPORTED)
		set_target_properties(WdVulkan::Loader PROPERTIES IMPORTED_LOCATION "${WD_VULKAN_DIR}/Lib/vulkan-1.lib")
		set_target_properties(WdVulkan::Loader PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${WD_VULKAN_DIR}/Include")

		add_library(WdVulkan::DXC SHARED IMPORTED)
		set_target_properties(WdVulkan::DXC PROPERTIES IMPORTED_LOCATION "${WD_VULKAN_DIR}/Bin/dxcompiler.dll")
		set_target_properties(WdVulkan::DXC PROPERTIES IMPORTED_IMPLIB "${WD_VULKAN_DIR}/Lib/dxcompiler.lib")
		set_target_properties(WdVulkan::DXC PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${WD_VULKAN_DIR}/Include")

	elseif(WD_CMAKE_PLATFORM_LINUX AND WD_CMAKE_ARCHITECTURE_64BIT)
		add_library(WdVulkan::Loader SHARED IMPORTED)
		set_target_properties(WdVulkan::Loader PROPERTIES IMPORTED_LOCATION "${WD_VULKAN_DIR}/x86_64/lib/libvulkan.so.1.3.216")
		set_target_properties(WdVulkan::Loader PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${WD_VULKAN_DIR}/x86_64/include")

		add_library(WdVulkan::DXC SHARED IMPORTED)
		set_target_properties(WdVulkan::DXC PROPERTIES IMPORTED_LOCATION "${WD_VULKAN_DIR}/x86_64/lib/libdxcompiler.so.3.7")
		set_target_properties(WdVulkan::DXC PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${WD_VULKAN_DIR}/x86_64/include")
	else()
		message(FATAL_ERROR "TODO: Vulkan is not yet supported on this platform and/or architecture.")
	endif()
endif()
