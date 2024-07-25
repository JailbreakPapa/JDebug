# #####################################
# ## Vulkan support
# #####################################

set(NS_BUILD_EXPERIMENTAL_VULKAN OFF CACHE BOOL "Whether to enable experimental / work-in-progress Vulkan code")

# #####################################
# ## ns_requires_vulkan()
# #####################################
macro(ns_requires_vulkan)
	ns_requires(NS_CMAKE_PLATFORM_SUPPORTS_VULKAN)
	ns_requires(NS_BUILD_EXPERIMENTAL_VULKAN)
	find_package(NsVulkan REQUIRED)
endmacro()

# #####################################
# ## ns_link_target_vulkan(<target>)
# #####################################
function(ns_link_target_vulkan TARGET_NAME)
	ns_requires_vulkan()


	if(NSVULKAN_FOUND)
		target_link_libraries(${TARGET_NAME} PRIVATE NsVulkan::Loader)

		if (COMMAND ns_platformhook_link_target_vulkan)
			# call platform-specific hook for linking with Vulkan
			ns_platformhook_link_target_vulkan()
		endif()
	endif()
endfunction()

# #####################################
# ## ns_link_target_dxc(<target>)
# #####################################
function(ns_link_target_dxc TARGET_NAME)
	ns_requires_vulkan()


	if(NSVULKAN_FOUND)
		target_link_libraries(${TARGET_NAME} PRIVATE NsVulkan::DXC)

		get_target_property(_dll_location NsVulkan::DXC IMPORTED_LOCATION)

		if(NOT _dll_location STREQUAL "")
			add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:NsVulkan::DXC> $<TARGET_FILE_DIR:${TARGET_NAME}>)
		endif()

		unset(_dll_location)
	endif()
endfunction()

# #####################################
# ## ns_sources_target_spirv_reflect(<target>)
# #####################################
function(ns_sources_target_spirv_reflect TARGET_NAME)
	ns_requires_vulkan()


	if(NSVULKAN_FOUND)
		if(NS_CMAKE_PLATFORM_WINDOWS_DESKTOP AND NS_CMAKE_ARCHITECTURE_64BIT)
			target_include_directories(${TARGET_NAME} PRIVATE "${NS_VULKAN_DIR}/Source/SPIRV-Reflect")
			target_sources(${TARGET_NAME} PRIVATE "${NS_VULKAN_DIR}/Source/SPIRV-Reflect/spirv_reflect.h")
			target_sources(${TARGET_NAME} PRIVATE "${NS_VULKAN_DIR}/Source/SPIRV-Reflect/spirv_reflect.c")
			source_group("SPIRV-Reflect" FILES "${NS_VULKAN_DIR}/Source/SPIRV-Reflect/spirv_reflect.h" "${NS_VULKAN_DIR}/x86_64/include/SPIRV-Reflect/spirv_reflect.c")
		elseif(NS_CMAKE_PLATFORM_LINUX AND NS_CMAKE_ARCHITECTURE_64BIT)
			target_include_directories(${TARGET_NAME} PRIVATE "${NS_VULKAN_DIR}/x86_64/include/SPIRV-Reflect")
			target_sources(${TARGET_NAME} PRIVATE "${NS_VULKAN_DIR}/x86_64/include/SPIRV-Reflect/spirv_reflect.h")
			target_sources(${TARGET_NAME} PRIVATE "${NS_VULKAN_DIR}/x86_64/include/SPIRV-Reflect/spirv_reflect.c")
			source_group("SPIRV-Reflect" FILES "${NS_VULKAN_DIR}/x86_64/include/SPIRV-Reflect/spirv_reflect.h" "${NS_VULKAN_DIR}/x86_64/include/SPIRV-Reflect/spirv_reflect.c")
		else()
			message(FATAL_ERROR "TODO: Vulkan is not yet supported on this platform and/or architecture.")
		endif()
	endif()
endfunction()