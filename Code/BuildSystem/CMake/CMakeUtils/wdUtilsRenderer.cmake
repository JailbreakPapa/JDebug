# #####################################
# ## wd_requires_renderer()
# #####################################

macro(wd_requires_renderer)
	if(WD_CMAKE_PLATFORM_WINDOWS)
		wd_requires_d3d()
	else()
		wd_requires_vulkan()
	endif()
endmacro()

# #####################################
# ## wd_add_renderers(<target>)
# ## Add all required libraries and dependencies to the given target so it has accedss to all available renderers.
# #####################################
function(wd_add_renderers TARGET_NAME)
	if(WD_BUILD_EXPERIMENTAL_VULKAN)
		target_link_libraries(${TARGET_NAME}
			PRIVATE
			RendererVulkan
		)

		add_dependencies(${TARGET_NAME}
			ShaderCompilerDXC
		)
	endif()

	if(WD_CMAKE_PLATFORM_WINDOWS)
		target_link_libraries(${TARGET_NAME}
			PRIVATE
			RendererDX11
		)
		wd_link_target_dx11(${TARGET_NAME})

		add_dependencies(${TARGET_NAME}
			ShaderCompilerHLSL
		)
	endif()
endfunction()