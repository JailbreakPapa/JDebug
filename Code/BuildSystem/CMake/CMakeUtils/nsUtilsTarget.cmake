# #####################################
# ## ns_create_target(<LIBRARY | APPLICATION> <target-name> [NO_PCH] [NO_UNITY] [NO_QT] [ALL_SYMBOLS_VISIBLE] [EXCLUDE_FOLDER_FOR_UNITY <relative-folder>...])
# #####################################

macro(ns_create_target TYPE TARGET_NAME)
	ns_apply_build_filter(${TARGET_NAME})

	set(ARG_OPTIONS NO_PCH NO_UNITY NO_QT NO_NS_PREFIX ENABLE_RTTI NO_WARNINGS_AS_ERRORS NO_COMPLIANCE ALL_SYMBOLS_VISIBLE NO_DEBUG)
	set(ARG_ONEVALUEARGS "")
	set(ARG_MULTIVALUEARGS EXCLUDE_FOLDER_FOR_UNITY EXCLUDE_FROM_PCH_REGEX MANUAL_SOURCE_FILES)
	cmake_parse_arguments(ARG "${ARG_OPTIONS}" "${ARG_ONEVALUEARGS}" "${ARG_MULTIVALUEARGS}" ${ARGN})

	if(ARG_UNPARSED_ARGUMENTS)
		message(FATAL_ERROR "ns_create_target: Invalid arguments '${ARG_UNPARSED_ARGUMENTS}'")
	endif()

	ns_pull_all_vars()

	if(DEFINED ARG_MANUAL_SOURCE_FILES)
		set(ALL_SOURCE_FILES ${ARG_MANUAL_SOURCE_FILES})
	else()
		ns_glob_source_files(${CMAKE_CURRENT_SOURCE_DIR} ALL_SOURCE_FILES)
	endif()

	# SHARED_LIBRARY means always shared
	# LIBRARY means SHARED_LIBRARY when NS_COMPILE_ENGINE_AS_DLL is on, otherwise STATIC_LIBRARY
	if((${TYPE} STREQUAL "LIBRARY") OR(${TYPE} STREQUAL "STATIC_LIBRARY") OR(${TYPE} STREQUAL "SHARED_LIBRARY"))
		if((${NS_COMPILE_ENGINE_AS_DLL} AND(${TYPE} STREQUAL "LIBRARY")) OR(${TYPE} STREQUAL "SHARED_LIBRARY"))
			message(STATUS "Shared Library: ${TARGET_NAME}")
			add_library(${TARGET_NAME} SHARED "${ALL_SOURCE_FILES}")

		else()
			message(STATUS "Static Library: ${TARGET_NAME}")
			add_library(${TARGET_NAME} STATIC "${ALL_SOURCE_FILES}")
		endif()

		if(ARG_NO_NS_PREFIX)
			# on some platforms like linux there is a default prefix like "lib".
			# We don't want that as it confuses our plugin system.
			set_target_properties(${TARGET_NAME} PROPERTIES IMPORT_PREFIX "")
			set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "")
		else()
			ns_add_output_ns_prefix(${TARGET_NAME})
		endif()

		# PLATFORM-TODO (use hook above?)
		ns_uwp_fix_library_properties(${TARGET_NAME} "${ALL_SOURCE_FILES}")

	elseif(${TYPE} STREQUAL "APPLICATION")
		message(STATUS "Application: ${TARGET_NAME}")

		# PLATFORM-TODO
		# On Android we can't use executables. Instead we have to use shared libraries which are loaded from java code.
		if(NS_CMAKE_PLATFORM_ANDROID)
			# All ns applications must include the native app glue implementation
			add_library(${TARGET_NAME} SHARED ${ALL_SOURCE_FILES} "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c")

			# Prevent the linker from stripping away the application entry point of android_native_app_glue: ANativeActivity_onCreate
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

			# The log and android libraries are library dependencies of android_native_app_glue
			target_link_libraries(${TARGET_NAME} PRIVATE log android EGL GLESv1_CM)
		else()
			add_executable(${TARGET_NAME} ${ALL_SOURCE_FILES})
		endif()

		# PLATFORM-TODO (use hook from below?)
		ns_uwp_add_default_content(${TARGET_NAME})

		if(COMMAND ns_platformhook_set_application_properties)
			ns_platformhook_set_application_properties(${TARGET_NAME})
		endif()

	else()
		message(FATAL_ERROR "ns_create_target: Missing argument to specify target type. Pass in 'APP' or 'LIB'.")
	endif()

	# sort files into the on-disk folder structure in the IDE
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ALL_SOURCE_FILES})
	# PCH Seems to lead to problems with Consoles, so we disable it for now.
	if(NOT ${ARG_NO_PCH} AND NOT NS_CMAKE_PLATFORM_CONSOLE)
		ns_auto_pch(${TARGET_NAME} "${ALL_SOURCE_FILES}" ${ARG_EXCLUDE_FROM_PCH_REGEX})
	endif()

	# When using the Open Folder workflow inside visual studio on android, visual studio gets confused due to our custom output directory
	# Do not set the custom output directory in this case
	if((NOT ANDROID) OR(NOT NS_CMAKE_INSIDE_VS))
		# PLATFORM-TODO
		ns_set_default_target_output_dirs(${TARGET_NAME})
	endif()

	# PLATFORM-TODO: add general hook ?

	# We need the target directory to add the apk packaging steps for android. Thus, this step needs to be done here.
	if(${TYPE} STREQUAL "APPLICATION")
		ns_android_add_default_content(${TARGET_NAME})
	endif()

	ns_add_target_folder_as_include_dir(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR})

	ns_set_common_target_definitions(${TARGET_NAME})

	ns_set_build_flags(${TARGET_NAME} ${ARGN})

	# On linux we want all symbols to be hidden by default. We manually "export" them.
	if(NS_COMPILE_ENGINE_AS_DLL AND NS_CMAKE_PLATFORM_LINUX AND NOT ARG_ALL_SYMBOLS_VISIBLE)
		# PLATFORM-TODO (use general hook as above?)
		target_compile_options(${TARGET_NAME} PRIVATE "-fvisibility=hidden")
	endif()

	ns_set_project_ide_folder(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR})

	# PLATFORM-TODO (use general hook as above?)
	# Pass the windows sdk include paths to the resource compiler when not generating a visual studio solution.
	if(NS_CMAKE_PLATFORM_WINDOWS AND NOT NS_CMAKE_GENERATOR_MSVC)
		set(RC_FILES ${ALL_SOURCE_FILES})
		list(FILTER RC_FILES INCLUDE REGEX ".*\\.rc$")

		if(RC_FILES)
			set_source_files_properties(${RC_FILES}
				PROPERTIES COMPILE_FLAGS "/I\"C:/Program Files (x86)/Windows Kits/10/Include/${NS_CMAKE_WINDOWS_SDK_VERSION}/shared\" /I\"C:/Program Files (x86)/Windows Kits/10/Include/${NS_CMAKE_WINDOWS_SDK_VERSION}/um\""
			)
		endif()
	endif()

	# PLATFORM-TODO (use general hook as above?)
	if(NS_CMAKE_PLATFORM_ANDROID)
		# Add the location for native_app_glue.h to the include directories.
		target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_ANDROID_NDK}/sources/android/native_app_glue")
	endif()

	if(NOT ${ARG_NO_QT})
		ns_qt_wrap_target_files(${TARGET_NAME} "${ALL_SOURCE_FILES}")
	endif()

	ns_ci_add_to_targets_list(${TARGET_NAME} C++)

	if(NOT ${ARG_NO_UNITY})
		ns_generate_folder_unity_files_for_target(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR} "${ARG_EXCLUDE_FOLDER_FOR_UNITY}")
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