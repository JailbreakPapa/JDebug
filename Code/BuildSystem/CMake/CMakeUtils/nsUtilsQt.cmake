# #####################################
# ## Qt support
# #####################################

set(NS_ENABLE_QT_SUPPORT ON CACHE BOOL "Whether to add Qt support.")
set(NS_QT_DIR $ENV{QTDIR} CACHE PATH "Directory of the Qt installation")

# #####################################
# ## ns_requires_qt()
# #####################################
macro(ns_requires_qt)
	ns_requires(NS_ENABLE_QT_SUPPORT)
endmacro()

# #####################################
# ## ns_find_qt()
# ## invoked once in the root cmake script to locate all requierd QT components.
# #####################################
macro(ns_find_qt)
	ns_prepare_find_qt()

	set(NS_QT_COMPONENTS 
		Widgets
		Core
		Gui
		Widgets
		Network
		Svg
	)

	# nsEngine requires at least Qt 6.3 because earlier versions have a bug which prevents the 3d viewport in the 
	# Editor from working correctly.
	SET(NS_REQUIRED_QT_VERSION "6.3")

	if(NS_ENABLE_QT_SUPPORT)
		if(NS_QT_DIR)
			find_package(Qt6 ${NS_REQUIRED_QT_VERSION} COMPONENTS ${NS_QT_COMPONENTS} REQUIRED PATHS ${NS_QT_DIR})
		else()
			find_package(Qt6 ${NS_REQUIRED_QT_VERSION} COMPONENTS ${NS_QT_COMPONENTS} REQUIRED)
		endif()
	endif()

	message(STATUS "Found Qt6 Version ${Qt6_VERSION} in ${Qt6_DIR}")
	
	mark_as_advanced(FORCE Qt6_DIR)
	mark_as_advanced(FORCE Qt6Core_DIR)
	mark_as_advanced(FORCE Qt6CoreTools_DIR)
	mark_as_advanced(FORCE Qt6Gui_DIR)
	mark_as_advanced(FORCE Qt6GuiTools_DIR)
	mark_as_advanced(FORCE Qt6Widgets_DIR)
	mark_as_advanced(FORCE Qt6WidgetsTools_DIR)
	mark_as_advanced(FORCE Qt6Network_DIR)
	mark_as_advanced(FORCE Qt6Svg_DIR)
	mark_as_advanced(FORCE Qt6EntryPointPrivate_DIR)
	mark_as_advanced(FORCE Qt6ZlibPrivate_DIR)
	mark_as_advanced(FORCE WINDEPLOYQT_EXECUTABLE)
	mark_as_advanced(FORCE QT_ADDITIONAL_HOST_PACKAGES_PREFIX_PATH)
	mark_as_advanced(FORCE QT_ADDITIONAL_PACKAGES_PREFIX_PATH)

	if (COMMAND ns_platformhook_find_qt)
		ns_platformhook_find_qt()
	endif()
	
endmacro()

# #####################################
# ## ns_prepare_find_qt()
# #####################################
function(ns_prepare_find_qt)
	set(NS_CACHED_QT_DIR "NS_CACHED_QT_DIR-NOTFOUND" CACHE STRING "")
	mark_as_advanced(NS_CACHED_QT_DIR FORCE)

	# #####################
	# # Download Qt package
	ns_pull_compiler_and_architecture_vars()
	ns_pull_platform_vars()
	ns_pull_config_vars()

	if (COMMAND ns_platformhook_download_qt)
		ns_platformhook_download_qt()
	endif()

	# # Download Qt package
	# #####################
	if(NOT "${NS_QT_DIR}" STREQUAL "${NS_CACHED_QT_DIR}")
		# Need to reset qt vars now so that 'find_package' is re-executed
		set(NS_CACHED_QT_DIR ${NS_QT_DIR} CACHE STRING "" FORCE)

		message(STATUS "Qt-dir has changed, clearing cached Qt paths")

		# Clear cached qt dirs
		set(Qt6_DIR "Qt6_DIR-NOTFOUND" CACHE PATH "" FORCE)
		set(Qt6Core_DIR "Qt6Core_DIR-NOTFOUND" CACHE PATH "" FORCE)
		set(Qt6CoreTools_DIR "Qt6CoreTools_DIR-NOTFOUND" CACHE PATH "" FORCE)
		set(Qt6Gui_DIR "Qt6Gui_DIR-NOTFOUND" CACHE PATH "" FORCE)
		set(Qt6GuiTools_DIR "Qt6GuiTools_DIR-NOTFOUND" CACHE PATH "" FORCE)
		set(Qt6Widgets_DIR "Qt6Widgets_DIR-NOTFOUND" CACHE PATH "" FORCE)
		set(Qt6WidgetsTools_DIR "Qt6WidgetTools_DIR-NOTFOUND" CACHE PATH "" FORCE)
		set(Qt6Network_DIR "Qt6Network_DIR-NOTFOUND" CACHE PATH "" FORCE)
		set(Qt6Svg_DIR "Qt6Svg_DIR-NOTFOUND" CACHE PATH "" FORCE)
		set(Qt6ZlibPrivate_DIR "Qt6ZlibPrivate_DIR-NOTFOUND" CACHE PATH "" FORCE)
		set(Qt6EntryPointPrivate_DIR "Qt6EntryPointPrivate_DIR-NOTFOUND" CACHE PATH "" FORCE)
		set(WINDEPLOYQT_EXECUTABLE "WINDEPLOYQT_EXECUTABLE-NOTFOUND" CACHE FILEPATH "" FORCE)
	endif()

	# force find_package to search for Qt in the correct folder
	if(NS_QT_DIR)
		set(CMAKE_PREFIX_PATH ${NS_QT_DIR} PARENT_SCOPE)
	endif()
endfunction()

# #####################################
# ## ns_link_target_qt(TARGET <target> COMPONENTS <qt components> [COPY_DLLS])
# #####################################
function(ns_link_target_qt)
	ns_pull_all_vars()

	ns_requires_qt()

	set(FN_OPTIONS COPY_DLLS)
	set(FN_ONEVALUEARGS TARGET)
	set(FN_MULTIVALUEARGS COMPONENTS)
	cmake_parse_arguments(FN_ARG "${FN_OPTIONS}" "${FN_ONEVALUEARGS}" "${FN_MULTIVALUEARGS}" ${ARGN})

	if(FN_ARG_UNPARSED_ARGUMENTS)
		message(FATAL_ERROR "ns_link_target_qt: Invalid arguments '${FN_ARG_UNPARSED_ARGUMENTS}'")
	endif()

	get_property(NS_SUBMODULE_PREFIX_PATH GLOBAL PROPERTY NS_SUBMODULE_PREFIX_PATH)

	file(RELATIVE_PATH SUB_FOLDER "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/..")

	target_include_directories(${FN_ARG_TARGET} PUBLIC ${CMAKE_BINARY_DIR}/${SUB_FOLDER})

	target_compile_definitions(${FN_ARG_TARGET} PUBLIC NS_USE_QT)

	if(NS_CMAKE_COMPILER_MSVC)
		# Qt6 requires runtime type information (RTTI) in debug builds.
		target_compile_options(${FN_ARG_TARGET} PRIVATE "$<$<CONFIG:Debug>:/GR>")
	endif()

	foreach(module ${FN_ARG_COMPONENTS})
		target_link_libraries(${FN_ARG_TARGET} PUBLIC "Qt6::${module}")

		if(FN_ARG_COPY_DLLS)
			# copy the dll into the binary folder for each configuration using generator expressions
			# as a post-build step for every qt-enabled target:
			add_custom_command(TARGET ${FN_ARG_TARGET} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
				$<TARGET_FILE:Qt6::${module}>
				$<TARGET_FILE_DIR:${FN_ARG_TARGET}>)
		endif()
	endforeach()

	if(NS_QT_DIR AND FN_ARG_COPY_DLLS)
		# Copy 'imageformats' into the binary folder.
		add_custom_command(TARGET ${FN_ARG_TARGET} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${NS_QT_DIR}/plugins/imageformats"
			"$<TARGET_FILE_DIR:${FN_ARG_TARGET}>/imageformats")

		# Copy 'platforms' into the binary folder.
		add_custom_command(TARGET ${FN_ARG_TARGET} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${NS_QT_DIR}/plugins/platforms"
			"$<TARGET_FILE_DIR:${FN_ARG_TARGET}>/platforms")

		# Copy 'iconengines' into the binary folder.
		add_custom_command(TARGET ${FN_ARG_TARGET} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${NS_QT_DIR}/plugins/iconengines"
			"$<TARGET_FILE_DIR:${FN_ARG_TARGET}>/iconengines")
	endif()
endfunction()

# #####################################
# ## ns_qt_wrap_target_ui_files(<target> <files>)
# #####################################
function(ns_qt_wrap_target_ui_files TARGET_NAME FILES_TO_WRAP)
	ns_requires_qt()

	list(FILTER FILES_TO_WRAP INCLUDE REGEX ".*\.ui$")

	if(NOT FILES_TO_WRAP)
		return()
	endif()

	if(NOT TARGET Qt6::uic)
		message(FATAL_ERROR "UIC.exe not found")
	else()
		# due to the stupid way CMake variable scopes work, and a stupid way Qt sets up the Qt6Widgets_UIC_EXECUTABLE variable,
		# its value gets 'lost' (reset to empty) if find_package has been called in a different function before
		# since it is used internally by Qt6_wrap_ui, we must ensure to restore it here, otherwise the custom build tool command on UI files
		# will miss the uic.exe part and just be "-o stuff" instead of "uic.exe -o stuff"
		get_target_property(Qt6Widgets_UIC_EXECUTABLE Qt6::uic IMPORTED_LOCATION)
	endif()

	Qt6_wrap_ui(UIC_FILES ${FILES_TO_WRAP})

	target_sources(${TARGET_NAME} PRIVATE ${UIC_FILES})

	source_group("Qt\\UI Files" FILES ${UIC_FILES})
endfunction()

# #####################################
# ## ns_qt_wrap_target_moc_files(<target> <files>)
# #####################################
function(ns_qt_wrap_target_moc_files TARGET_NAME FILES_TO_WRAP)
	ns_requires_qt()

	list(FILTER FILES_TO_WRAP INCLUDE REGEX ".*\.h$")

	if(NOT FILES_TO_WRAP)
		return()
	endif()

	set(Qt6Core_MOC_EXECUTABLE Qt6::moc)
	ns_retrieve_target_pch(${TARGET_NAME} PCH_H)

	if(PCH_H)
		Qt6_wrap_cpp(MOC_FILES TARGET ${TARGET_NAME} ${FILES_TO_WRAP} OPTIONS -b "${PCH_H}.h")
		ns_pch_use("${PCH_H}.h" "${MOC_FILES}")
	else()
		Qt6_wrap_cpp(MOC_FILES TARGET ${TARGET_NAME} ${FILES_TO_WRAP})
	endif()

	target_sources(${TARGET_NAME} PRIVATE ${MOC_FILES})

	source_group("Qt\\MOC Files" FILES ${MOC_FILES})
endfunction()

# #####################################
# ## ns_qt_wrap_target_qrc_files(<target> <files>)
# #####################################
function(ns_qt_wrap_target_qrc_files TARGET_NAME FILES_TO_WRAP)
	ns_requires_qt()

	list(FILTER FILES_TO_WRAP INCLUDE REGEX ".*\.qrc$")

	if(NOT FILES_TO_WRAP)
		return()
	endif()

	set(Qt6Core_RCC_EXECUTABLE Qt6::rcc)
	Qt6_add_resources(QRC_FILES ${FILES_TO_WRAP})

	target_sources(${TARGET_NAME} PRIVATE ${QRC_FILES})

	source_group("Qt\\QRC Files" FILES ${QRC_FILES})
endfunction()

# #####################################
# ## ns_qt_wrap_target_files(<target> <files>)
# #####################################
function(ns_qt_wrap_target_files TARGET_NAME FILES_TO_WRAP)
	ns_requires_qt()

	ns_qt_wrap_target_qrc_files(${TARGET_NAME} "${FILES_TO_WRAP}")

	ns_qt_wrap_target_ui_files(${TARGET_NAME} "${FILES_TO_WRAP}")

	# in this automated method, we only MOC files that end with ".moc.h"
	list(FILTER FILES_TO_WRAP INCLUDE REGEX ".*\.moc\.h")
	ns_qt_wrap_target_moc_files(${TARGET_NAME} "${FILES_TO_WRAP}")
endfunction()
