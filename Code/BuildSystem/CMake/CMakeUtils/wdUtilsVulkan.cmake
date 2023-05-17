# #####################################
# ## Vulkan support
# #####################################

set(WD_BUILD_EXPERIMENTAL_VULKAN OFF CACHE BOOL "Whether to enable experimental / work-in-progress Vulkan code")

# #####################################
# ## wd_requires_vulkan()
# #####################################
macro(wd_requires_vulkan)
	wd_requires_one_of(WD_CMAKE_PLATFORM_LINUX WD_CMAKE_PLATFORM_WINDOWS)
	wd_requires(WD_BUILD_EXPERIMENTAL_VULKAN)
	find_package(WdVulkan REQUIRED)
endmacro()

# #####################################
# ## wd_link_target_vulkan(<target>)
# #####################################
function(wd_link_target_vulkan TARGET_NAME)
	wd_requires_vulkan()

	find_package(WdVulkan REQUIRED)

	if(WDVULKAN_FOUND)
		target_link_libraries(${TARGET_NAME} PRIVATE WdVulkan::Loader)

		# Only on linux is the loader a dll.
		if(WD_CMAKE_PLATFORM_LINUX)
			get_target_property(_dll_location WdVulkan::Loader IMPORTED_LOCATION)

			if(NOT _dll_location STREQUAL "")
				add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:WdVulkan::Loader> $<TARGET_FILE_DIR:${TARGET_NAME}>)
			endif()

			unset(_dll_location)
		endif()
	endif()
endfunction()

# #####################################
# ## wd_link_target_dxc(<target>)
# #####################################
function(wd_link_target_dxc TARGET_NAME)
	wd_requires_vulkan()

	find_package(WdVulkan REQUIRED)

	if(WDVULKAN_FOUND)
		target_link_libraries(${TARGET_NAME} PRIVATE WdVulkan::DXC)

		get_target_property(_dll_location WdVulkan::DXC IMPORTED_LOCATION)

		if(NOT _dll_location STREQUAL "")
			add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:WdVulkan::DXC> $<TARGET_FILE_DIR:${TARGET_NAME}>)
		endif()

		unset(_dll_location)
	endif()
endfunction()

# #####################################
# ## wd_sources_target_spirv_reflect(<target>)
# #####################################
function(wd_sources_target_spirv_reflect TARGET_NAME)
	wd_requires_vulkan()

	find_package(WdVulkan REQUIRED)

	if(WDVULKAN_FOUND)
		target_include_directories(${TARGET_NAME} PRIVATE "${WD_VULKAN_DIR}/source/SPIRV-Reflect")
		target_sources(${TARGET_NAME} PRIVATE "${WD_VULKAN_DIR}/source/SPIRV-Reflect/spirv_reflect.h")
		target_sources(${TARGET_NAME} PRIVATE "${WD_VULKAN_DIR}/source/SPIRV-Reflect/spirv_reflect.c")
		source_group("SPIRV-Reflect" FILES "${WD_VULKAN_DIR}/source/SPIRV-Reflect/spirv_reflect.h" "${WD_VULKAN_DIR}/source/SPIRV-Reflect/spirv_reflect.c")
	endif()
endfunction()