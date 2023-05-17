# #####################################
# ## Embree support
# #####################################

set(WD_BUILD_EMBREE OFF CACHE BOOL "Whether support for Intel Embree should be added")

# #####################################
# ## wd_requires_embree()
# #####################################
macro(wd_requires_embree)
	wd_requires_windows()
	wd_requires(WD_BUILD_EMBREE)
endmacro()

# #####################################
# ## wd_link_target_embree(<target>)
# #####################################
function(wd_link_target_embree TARGET_NAME)
	wd_requires_embree()

	find_package(WdEmbree REQUIRED)

	if(WDEMBREE_FOUND)
		target_link_libraries(${TARGET_NAME} PRIVATE WdEmbree::WdEmbree)

		target_compile_definitions(${PROJECT_NAME} PUBLIC BUILDSYSTEM_ENABLE_EMBREE_SUPPORT)

		add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:WdEmbree::WdEmbree> $<TARGET_FILE_DIR:${TARGET_NAME}>
		)
	endif()
endfunction()
