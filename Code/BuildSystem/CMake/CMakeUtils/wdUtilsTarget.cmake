# #####################################
# ## wd_create_target(<LIBRARY | APPLICATION > <target-name> [NO_PCH] [THIRD_PARTY] [NO_UNITY] [NO_QT] [ALL_SYMBOLS_VISIBLE] [EXCLUDE_FOLDER_FOR_UNITY <relative-folder>...])
# #####################################
# NOTE: Comment out RTEngine stuff, we dont need the endtargetname rubbish.
macro(wd_create_target TYPE TARGET_NAME)
	wd_apply_build_filter(${TARGET_NAME})

	set(ARG_OPTIONS NO_PCH NO_UNITY NO_QT NO_WD_PREFIX ENABLE_RTTI NO_WARNINGS_AS_ERRORS NO_CONTROLFLOWGUARD ALL_SYMBOLS_VISIBLE NO_DEBUG THIRD_PARTY)
	set(ARG_ONEVALUEARGS "")
	set(ARG_MULTIVALUEARGS EXCLUDE_FOLDER_FOR_UNITY EXCLUDE_FROM_PCH_REGEX MANUAL_SOURCE_FILES)
	cmake_parse_arguments(ARG "${ARG_OPTIONS}" "${ARG_ONEVALUEARGS}" "${ARG_MULTIVALUEARGS}" ${ARGN})

	if(ARG_UNPARSED_ARGUMENTS)
		message(FATAL_ERROR "wd_create_target: Invalid arguments '${ARG_UNPARSED_ARGUMENTS}'")
	endif()

	wd_pull_all_vars()

	if(DEFINED ARG_MANUAL_SOURCE_FILES)
		set(ALL_SOURCE_FILES ${ARG_MANUAL_SOURCE_FILES})
	else()
		wd_glob_source_files(${CMAKE_CURRENT_SOURCE_DIR} ALL_SOURCE_FILES)
	endif()

	# SHARED_LIBRARY means always shared
	# LIBRARY means SHARED_LIBRARY when WD_COMPILE_ENGINE_AS_DLL is on, otherwise STATIC_LIBRARY
	if((${TYPE} STREQUAL "LIBRARY") OR(${TYPE} STREQUAL "STATIC_LIBRARY") OR(${TYPE} STREQUAL "SHARED_LIBRARY"))
        if((${WD_COMPILE_ENGINE_AS_DLL} AND(${TYPE} STREQUAL "LIBRARY")) OR(${TYPE} STREQUAL "SHARED_LIBRARY"))
			# set(END_TARGET_NAME "Engine.${CMAKE_HOST_SYSTEM_NAME}.${TARGET_NAME}")
			message(STATUS "Shared Library: ${TARGET_NAME} ")
			add_library(${TARGET_NAME} SHARED "${ALL_SOURCE_FILES}")
            #if(NOT ${ARG_THIRD_PARTY})
           	#     set_target_properties(${TARGET_NAME}
           	#         PROPERTIES
           	#         OUTPUT_NAME ${END_TARGET_NAME}
           	#     )
            #endif()
		else()
			#set(END_TARGET_NAME "Engine.${CMAKE_HOST_SYSTEM_NAME}.${TARGET_NAME}")
			message(STATUS "Static Library: ${TARGET_NAME} ")
			add_library(${TARGET_NAME} STATIC "${ALL_SOURCE_FILES}")
            #if(NOT ${ARG_THIRD_PARTY})
            #    set_target_properties(${TARGET_NAME}
            #        PROPERTIES
            #        OUTPUT_NAME ${END_TARGET_NAME}
            #    )
            #endif()
		endif()

		if(NOT ARG_NO_WD_PREFIX)
			wd_add_output_wd_prefix(${TARGET_NAME})
		endif()

		wd_set_library_properties(${TARGET_NAME})
		wd_uwp_fix_library_properties(${TARGET_NAME} "${ALL_SOURCE_FILES}")

	elseif(${TYPE} STREQUAL "APPLICATION")
		message(STATUS "Application: ${TARGET_NAME}")

        add_executable(${TARGET_NAME} ${ALL_SOURCE_FILES})

        if(WD_ALLOW_LIVEPP_COMPILER_SUPPORT)
            # Compiler Settings needed in order for Live++ to work.
            target_compile_options(${TARGET_NAME} PRIVATE /Z7 /Zi /Gm- /Gy /Gw)
            target_link_options(${TARGET_NAME} PRIVATE /FUNCTIONPADMIN /OPT:NOREF /OPT:NOICF /DEBUG:FULL)
        endif()
		wd_uwp_add_default_content(${TARGET_NAME})

		wd_set_application_properties(${TARGET_NAME})

	else()
		message(FATAL_ERROR "wd_create_target: Missing argument to specify target type. Pass in 'APP' or 'LIB'.")
	endif()

	# sort files into the on-disk folder structure in the IDE
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ALL_SOURCE_FILES})

	if(NOT ${ARG_NO_PCH})
		wd_auto_pch(${TARGET_NAME} "${ALL_SOURCE_FILES}" ${ARG_EXCLUDE_FROM_PCH_REGEX})
	endif()

	# When using the Open Folder workflow inside visual studio on android, visual studio gets confused due to our custom output directory
	# Do not set the custom output directory in this case
	if((NOT ANDROID) OR(NOT WD_CMAKE_INSIDE_VS))
		wd_set_default_target_output_dirs(${TARGET_NAME})
	endif()


	wd_add_target_folder_as_include_dir(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR})

	wd_set_common_target_definitions(${TARGET_NAME})

	wd_set_build_flags(${TARGET_NAME} ${ARGN})

	# On linux we want all symbols to be hidden by default. We manually "export" them.
	if(WD_COMPILE_ENGINE_AS_DLL AND WD_CMAKE_PLATFORM_LINUX AND NOT ARG_ALL_SYMBOLS_VISIBLE)
		target_compile_options(${TARGET_NAME} PRIVATE "-fvisibility=hidden")
	endif()

	wd_set_project_ide_folder(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR})

	# Pass the windows sdk include paths to the resource compiler when not generating a visual studio solution.
	if(WD_CMAKE_PLATFORM_WINDOWS AND NOT WD_CMAKE_GENERATOR_MSVC)
		set(RC_FILES ${ALL_SOURCE_FILES})
		list(FILTER RC_FILES INCLUDE REGEX ".*\\.rc$")

		if(RC_FILES)
			set_source_files_properties(${RC_FILES}
				PROPERTIES COMPILE_FLAGS "/I\"C:/Program Files (x86)/Windows Kits/10/Include/${WD_CMAKE_WINDOWS_SDK_VERSION}/shared\" /I\"C:/Program Files (x86)/Windows Kits/10/Include/${WD_CMAKE_WINDOWS_SDK_VERSION}/um\""
			)
		endif()
	endif()

	if(WD_CMAKE_PLATFORM_ANDROID)
		# Add the location for native_app_glue.h to the include directories.
		target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue")
	endif()

	if(NOT ${ARG_NO_QT})
		wd_qt_wrap_target_files(${TARGET_NAME} "${ALL_SOURCE_FILES}")
	endif()

	wd_ci_add_to_targets_list(${TARGET_NAME} C++)

	if(NOT ${ARG_NO_UNITY})
		wd_generate_folder_unity_files_for_target(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR} "${ARG_EXCLUDE_FOLDER_FOR_UNITY}")
	endif()

	get_property(GATHER_EXTERNAL_PROJECTS GLOBAL PROPERTY "GATHER_EXTERNAL_PROJECTS")

	if(GATHER_EXTERNAL_PROJECTS)
		set_property(GLOBAL APPEND PROPERTY "EXTERNAL_PROJECTS" ${TARGET_NAME})
	endif()

	get_property(GATHER_EXPORT_PROJECTS GLOBAL PROPERTY "GATHER_EXPORT_PROJECTS")

	if(GATHER_EXPORT_PROJECTS)
		set_property(GLOBAL APPEND PROPERTY "EXPORT_PROJECTS" ${TARGET_NAME})
	endif()

endmacro()