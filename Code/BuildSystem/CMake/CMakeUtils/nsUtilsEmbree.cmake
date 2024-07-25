# #####################################
# ## Embree support
# #####################################

set(NS_BUILD_EMBREE OFF CACHE BOOL "Whether support for Intel Embree should be added")

# #####################################
# ## ns_requires_embree()
# #####################################
macro(ns_requires_embree)
	ns_requires(NS_CMAKE_PLATFORM_WINDOWS)
	ns_requires(NS_BUILD_EMBREE)
endmacro()

# #####################################
# ## ns_link_target_embree(<target>)
# #####################################
function(ns_link_target_embree TARGET_NAME)
	ns_requires_embree()

	find_package(NsEmbree REQUIRED)

	if(NSEMBREE_FOUND)
		target_link_libraries(${TARGET_NAME} PRIVATE NsEmbree::NsEmbree)

		target_compile_definitions(${PROJECT_NAME} PUBLIC BUILDSYSTEM_ENABLE_EMBREE_SUPPORT)

		add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:NsEmbree::NsEmbree> $<TARGET_FILE_DIR:${TARGET_NAME}>
		)
	endif()
endfunction()
