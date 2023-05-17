# #####################################
# ## wd_detect_project_name(<out-name>)
# #####################################

function(wd_detect_project_name OUT_NAME)
	# unfortunately this has to be known before the PROJECT command,
	# but platform and compiler settings are only detected by CMake AFTER the project command
	# CMAKE_GENERATOR is the only value available before that, so we have to regex this a bit to
	# generate a useful name
	# thus, only VS solutions currently get nice names
	cmake_path(IS_PREFIX CMAKE_SOURCE_DIR ${CMAKE_BINARY_DIR} NORMALIZE IS_IN_SOURCE_BUILD)

	get_filename_component(NAME_REPO ${CMAKE_SOURCE_DIR} NAME)
	get_filename_component(NAME_DEST ${CMAKE_BINARY_DIR} NAME)

	set(DETECTED_NAME "${NAME_REPO}")

	if(NOT ${NAME_REPO} STREQUAL ${NAME_DEST})
		set(DETECTED_NAME "${DETECTED_NAME}_${NAME_DEST}")
	endif()

	set(${OUT_NAME} "${DETECTED_NAME}" PARENT_SCOPE)

	message(STATUS "Auto-detected solution name: ${DETECTED_NAME} (Generator = ${CMAKE_GENERATOR})")
endfunction()

# #####################################
# ## wd_detect_platform()
# #####################################
function(wd_detect_platform)
	get_property(PREFIX GLOBAL PROPERTY WD_CMAKE_PLATFORM_PREFIX)

	if(PREFIX)
		# has already run before and WD_CMAKE_PLATFORM_PREFIX is already set
		# message (STATUS "Redundant call to wd_detect_platform()")
		return()
	endif()

	set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_PREFIX "")
	set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_WINDOWS OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_WINDOWS_DESKTOP OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_WINDOWS_UWP OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_WINDOWS_7 OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_POSIX OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_OSX OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_LINUX OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_ANDROID OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_EMSCRIPTEN OFF)

	message(STATUS "CMAKE_SYSTEM_NAME is '${CMAKE_SYSTEM_NAME}'")

	if(EMSCRIPTEN)
		message(STATUS "Platform is Emscripten (WD_CMAKE_PLATFORM_EMSCRIPTEN)")

		set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_EMSCRIPTEN ON)

		set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_PREFIX "Web")

	elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows") # Desktop Windows
		message(STATUS "Platform is Windows (WD_CMAKE_PLATFORM_WINDOWS, WD_CMAKE_PLATFORM_WINDOWS_DESKTOP)")
		message(STATUS "CMAKE_SYSTEM_VERSION is ${CMAKE_SYSTEM_VERSION}")
		message(STATUS "CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION is ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")

		set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_WINDOWS ON)
		set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_WINDOWS_DESKTOP ON)

		set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_PREFIX "Win")

		if(${CMAKE_SYSTEM_VERSION} EQUAL 6.1)
			set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_WINDOWS_7 ON)
		endif()

	elseif(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore") # Windows Universal
		message(STATUS "Platform is Windows Universal (WD_CMAKE_PLATFORM_WINDOWS, WD_CMAKE_PLATFORM_WINDOWS_UWP)")

		set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_WINDOWS ON)
		set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_WINDOWS_UWP ON)

		set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_PREFIX "WinUWP")

	elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CURRENT_OSX_VERSION) # OS X
		message(STATUS "Platform is OS X (WD_CMAKE_PLATFORM_OSX, WD_CMAKE_PLATFORM_POSIX)")

		set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_POSIX ON)
		set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_OSX ON)

		set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_PREFIX "Osx")

	elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux") # Linux
		message(STATUS "Platform is Linux (WD_CMAKE_PLATFORM_LINUX, WD_CMAKE_PLATFORM_POSIX)")

		set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_POSIX ON)
		set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_LINUX ON)

		set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_PREFIX "Linux")

	elseif(CMAKE_SYSTEM_NAME STREQUAL "Android") # Android
		message(STATUS "Platform is Android (WD_CMAKE_PLATFORM_ANDROID, WD_CMAKE_PLATFORM_POSIX)")

		set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_POSIX ON)
		set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_ANDROID ON)

		set_property(GLOBAL PROPERTY WD_CMAKE_PLATFORM_PREFIX "Android")

	else()
		message(FATAL_ERROR "Platform '${CMAKE_SYSTEM_NAME}' is not supported! Please extend wd_detect_platform().")
	endif()

	get_property(WD_CMAKE_PLATFORM_WINDOWS GLOBAL PROPERTY WD_CMAKE_PLATFORM_WINDOWS)

	if(WD_CMAKE_PLATFORM_WINDOWS)
		if(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
			set(WD_CMAKE_WINDOWS_SDK_VERSION ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
		else()
			set(WD_CMAKE_WINDOWS_SDK_VERSION ${CMAKE_SYSTEM_VERSION})
			string(REGEX MATCHALL "\\." NUMBER_OF_DOTS "${WD_CMAKE_WINDOWS_SDK_VERSION}")
			list(LENGTH NUMBER_OF_DOTS NUMBER_OF_DOTS)

			if(NUMBER_OF_DOTS EQUAL 2)
				set(WD_CMAKE_WINDOWS_SDK_VERSION "${WD_CMAKE_WINDOWS_SDK_VERSION}.0")
			endif()
		endif()

		set_property(GLOBAL PROPERTY WD_CMAKE_WINDOWS_SDK_VERSION ${WD_CMAKE_WINDOWS_SDK_VERSION})
	endif()
endfunction()

# #####################################
# ## wd_pull_platform_vars()
# #####################################
macro(wd_pull_platform_vars)
	wd_detect_platform()

	get_property(WD_CMAKE_PLATFORM_PREFIX GLOBAL PROPERTY WD_CMAKE_PLATFORM_PREFIX)
	get_property(WD_CMAKE_PLATFORM_WINDOWS GLOBAL PROPERTY WD_CMAKE_PLATFORM_WINDOWS)
	get_property(WD_CMAKE_PLATFORM_WINDOWS_UWP GLOBAL PROPERTY WD_CMAKE_PLATFORM_WINDOWS_UWP)
	get_property(WD_CMAKE_PLATFORM_WINDOWS_DESKTOP GLOBAL PROPERTY WD_CMAKE_PLATFORM_WINDOWS_DESKTOP)
	get_property(WD_CMAKE_PLATFORM_WINDOWS_7 GLOBAL PROPERTY WD_CMAKE_PLATFORM_WINDOWS_7)
	get_property(WD_CMAKE_PLATFORM_POSIX GLOBAL PROPERTY WD_CMAKE_PLATFORM_POSIX)
	get_property(WD_CMAKE_PLATFORM_OSX GLOBAL PROPERTY WD_CMAKE_PLATFORM_OSX)
	get_property(WD_CMAKE_PLATFORM_LINUX GLOBAL PROPERTY WD_CMAKE_PLATFORM_LINUX)
	get_property(WD_CMAKE_PLATFORM_ANDROID GLOBAL PROPERTY WD_CMAKE_PLATFORM_ANDROID)
	get_property(WD_CMAKE_PLATFORM_EMSCRIPTEN GLOBAL PROPERTY WD_CMAKE_PLATFORM_EMSCRIPTEN)

	if(WD_CMAKE_PLATFORM_WINDOWS)
		get_property(WD_CMAKE_WINDOWS_SDK_VERSION GLOBAL PROPERTY WD_CMAKE_WINDOWS_SDK_VERSION)
	endif()
endmacro()

# #####################################
# ## wd_detect_generator()
# #####################################
function(wd_detect_generator)
	get_property(PREFIX GLOBAL PROPERTY WD_CMAKE_GENERATOR_PREFIX)

	if(PREFIX)
		# has already run before and WD_CMAKE_GENERATOR_PREFIX is already set
		# message (STATUS "Redundant call to wd_detect_generator()")
		return()
	endif()

	wd_pull_platform_vars()

	set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_PREFIX "")
	set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_CONFIGURATION "undefined")
	set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_MSVC OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_XCODE OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_MAKE OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_NINJA OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_INSIDE_VS OFF) # if cmake is called through the visual studio open folder workflow

	message(STATUS "CMAKE_VERSION is '${CMAKE_VERSION}'")
	message(STATUS "CMAKE_BUILD_TYPE is '${CMAKE_BUILD_TYPE}'")

	string(FIND ${CMAKE_VERSION} "MSVC" VERSION_CONTAINS_MSVC)

	if(${VERSION_CONTAINS_MSVC} GREATER -1)
		message(STATUS "CMake was called from Visual Studio Open Folder workflow")
		set_property(GLOBAL PROPERTY WD_CMAKE_INSIDE_VS ON)
	endif()

	message(STATUS "CMAKE_GENERATOR is '${CMAKE_GENERATOR}'")

	if(WD_CMAKE_PLATFORM_WINDOWS) # Supported windows generators
		if(CMAKE_GENERATOR MATCHES "Visual Studio")
			# Visual Studio (All VS generators define MSVC)
			message(STATUS "Generator is MSVC (WD_CMAKE_GENERATOR_MSVC)")

			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_MSVC ON)
			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_PREFIX "Vs")
			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_CONFIGURATION $<CONFIGURATION>)
		elseif(CMAKE_GENERATOR MATCHES "Ninja") # Ninja makefiles. Only makefile format supported by Visual Studio Open Folder
			message(STATUS "Buildsystem is Ninja (WD_CMAKE_GENERATOR_NINJA)")

			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_NINJA ON)
			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_PREFIX "Ninja")
			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_CONFIGURATION ${CMAKE_BUILD_TYPE})
		else()
			message(FATAL_ERROR "Generator '${CMAKE_GENERATOR}' is not supported on Windows! Please extend wd_detect_generator()")
		endif()

	elseif(WD_CMAKE_PLATFORM_OSX) # Supported OSX generators
		if(CMAKE_GENERATOR MATCHES "Xcode") # XCODE
			message(STATUS "Buildsystem is Xcode (WD_CMAKE_GENERATOR_XCODE)")

			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_XCODE ON)
			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_PREFIX "Xcode")
			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_CONFIGURATION $<CONFIGURATION>)

		elseif(CMAKE_GENERATOR MATCHES "Unix Makefiles") # Unix Makefiles (for QtCreator etc.)
			message(STATUS "Buildsystem is Make (WD_CMAKE_GENERATOR_MAKE)")

			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_MAKE ON)
			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_PREFIX "Make")
			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_CONFIGURATION ${CMAKE_BUILD_TYPE})

		else()
			message(FATAL_ERROR "Generator '${CMAKE_GENERATOR}' is not supported on OS X! Please extend wd_detect_generator()")
		endif()

	elseif(WD_CMAKE_PLATFORM_LINUX)
		if(CMAKE_GENERATOR MATCHES "Unix Makefiles") # Unix Makefiles (for QtCreator etc.)
			message(STATUS "Buildsystem is Make (WD_CMAKE_GENERATOR_MAKE)")

			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_MAKE ON)
			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_PREFIX "Make")
			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_CONFIGURATION ${CMAKE_BUILD_TYPE})

		elseif(CMAKE_GENERATOR MATCHES "Ninja" OR CMAKE_GENERATOR MATCHES "Ninja Multi-Config")
			message(STATUS "Buildsystem is Ninja (WD_CMAKE_GENERATOR_NINJA)")

			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_NINJA ON)
			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_PREFIX "Ninja")
			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_CONFIGURATION ${CMAKE_BUILD_TYPE})
		else()
			message(FATAL_ERROR "Generator '${CMAKE_GENERATOR}' is not supported on Linux! Please extend wd_detect_generator()")
		endif()

	elseif(WD_CMAKE_PLATFORM_ANDROID)
		if(CMAKE_GENERATOR MATCHES "Ninja" OR CMAKE_GENERATOR MATCHES "Ninja Multi-Config")
			message(STATUS "Buildsystem is Ninja (WD_CMAKE_GENERATOR_NINJA)")

			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_NINJA ON)
			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_PREFIX "Ninja")
			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_CONFIGURATION ${CMAKE_BUILD_TYPE})

		else()
			message(FATAL_ERROR "Generator '${CMAKE_GENERATOR}' is not supported on Android! Please extend wd_detect_generator()")
		endif()

	elseif(WD_CMAKE_PLATFORM_EMSCRIPTEN)
		if(CMAKE_GENERATOR MATCHES "Ninja" OR CMAKE_GENERATOR MATCHES "Ninja Multi-Config")
			message(STATUS "Buildsystem is Ninja (WD_CMAKE_GENERATOR_NINJA)")

			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_NINJA ON)
			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_PREFIX "Ninja")
			set_property(GLOBAL PROPERTY WD_CMAKE_GENERATOR_CONFIGURATION ${CMAKE_BUILD_TYPE})

		else()
			message(FATAL_ERROR "Generator '${CMAKE_GENERATOR}' is not supported on Emscripten! Please extend wd_detect_generator()")
		endif()

	else()
		message(FATAL_ERROR "Platform '${CMAKE_SYSTEM_NAME}' has not set up the supported generators. Please extend wd_detect_generator()")
	endif()
endfunction()

# #####################################
# ## wd_pull_generator_vars()
# #####################################
macro(wd_pull_generator_vars)
	wd_detect_generator()

	get_property(WD_CMAKE_GENERATOR_PREFIX GLOBAL PROPERTY WD_CMAKE_GENERATOR_PREFIX)
	get_property(WD_CMAKE_GENERATOR_CONFIGURATION GLOBAL PROPERTY WD_CMAKE_GENERATOR_CONFIGURATION)
	get_property(WD_CMAKE_GENERATOR_MSVC GLOBAL PROPERTY WD_CMAKE_GENERATOR_MSVC)
	get_property(WD_CMAKE_GENERATOR_XCODE GLOBAL PROPERTY WD_CMAKE_GENERATOR_XCODE)
	get_property(WD_CMAKE_GENERATOR_MAKE GLOBAL PROPERTY WD_CMAKE_GENERATOR_MAKE)
	get_property(WD_CMAKE_GENERATOR_NINJA GLOBAL PROPERTY WD_CMAKE_GENERATOR_NINJA)
	get_property(WD_CMAKE_INSIDE_VS GLOBAL PROPERTY WD_CMAKE_INSIDE_VS)
endmacro()

# #####################################
# ## wd_detect_compiler_and_architecture()
# #####################################
function(wd_detect_compiler_and_architecture)
	get_property(PREFIX GLOBAL PROPERTY WD_CMAKE_COMPILER_POSTFIX)

	if(PREFIX)
		# has already run before and WD_CMAKE_COMPILER_POSTFIX is already set
		# message (STATUS "Redundant call to wd_detect_compiler()")
		return()
	endif()

	wd_pull_platform_vars()
	wd_pull_generator_vars()
	wd_pull_config_vars()
	get_property(GENERATOR_MSVC GLOBAL PROPERTY WD_CMAKE_GENERATOR_MSVC)

	set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_POSTFIX "")
	set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_MSVC OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_MSVC_140 OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_MSVC_141 OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_MSVC_142 OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_MSVC_143 OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_CLANG OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_GCC OFF)

	set(FILE_TO_COMPILE "${CMAKE_SOURCE_DIR}/${WD_SUBMODULE_PREFIX_PATH}/${WD_CMAKE_RELPATH}/ProbingSrc/ArchitectureDetect.c")
	
	if (WD_SDK_DIR)
		set(FILE_TO_COMPILE "${WD_SDK_DIR}/${WD_CMAKE_RELPATH}/ProbingSrc/ArchitectureDetect.c")
	endif()

	# Only compile the detect file if we don't have a cached result from the last run
	if((NOT WD_DETECTED_COMPILER) OR (NOT WD_DETECTED_ARCH) OR (NOT WD_DETECTED_MSVC_VER))
		set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
		try_compile(COMPILE_RESULT
			${CMAKE_CURRENT_BINARY_DIR}
			${FILE_TO_COMPILE}
			OUTPUT_VARIABLE COMPILE_OUTPUT
		)

		if(NOT COMPILE_RESULT)
			message(FATAL_ERROR "Failed to detect compiler / target architecture. Compiler output: ${COMPILE_OUTPUT}")
		endif()

		if(${COMPILE_OUTPUT} MATCHES "ARCH:'([^']*)'")
			set(WD_DETECTED_ARCH ${CMAKE_MATCH_1} CACHE INTERNAL "")
		else()
			message(FATAL_ERROR "The compile test did not output the architecture. Compiler broken? Compiler output: ${COMPILE_OUTPUT}")
		endif()

		if(${COMPILE_OUTPUT} MATCHES "COMPILER:'([^']*)'")
			set(WD_DETECTED_COMPILER ${CMAKE_MATCH_1} CACHE INTERNAL "")
		else()
			message(FATAL_ERROR "The compile test did not output the compiler. Compiler broken? Compiler output: ${COMPILE_OUTPUT}")
		endif()
		
		if(WD_DETECTED_COMPILER STREQUAL "msvc")
			if(${COMPILE_OUTPUT} MATCHES "MSC_VER:'([^']*)'")
				set(WD_DETECTED_MSVC_VER ${CMAKE_MATCH_1} CACHE INTERNAL "")
			else()
				message(FATAL_ERROR "The compile test did not output the MSC_VER. Compiler broken? Compiler output: ${COMPILE_OUTPUT}")
			endif()
		else()
			set(WD_DETECTED_MSVC_VER "<NOT USING MSVC>" CACHE INTERNAL "")
		endif()
	endif()

	if(WD_DETECTED_COMPILER STREQUAL "msvc") # Visual Studio Compiler
		message(STATUS "Compiler is MSVC (WD_CMAKE_COMPILER_MSVC) version ${WD_DETECTED_MSVC_VER}")

		set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_MSVC ON)

		if(WD_DETECTED_MSVC_VER GREATER_EQUAL 1930)
			message(STATUS "Compiler is Visual Studio 2022 (WD_CMAKE_COMPILER_MSVC_143)")
			set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_MSVC_143 ON)
			set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_POSTFIX "2022")

		elseif(WD_DETECTED_MSVC_VER GREATER_EQUAL 1920)
			message(STATUS "Compiler is Visual Studio 2019 (WD_CMAKE_COMPILER_MSVC_142)")
			set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_MSVC_142 ON)
			set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_POSTFIX "2019")

		elseif(WD_DETECTED_MSVC_VER GREATER_EQUAL 1910)
			message(STATUS "Compiler is Visual Studio 2017 (WD_CMAKE_COMPILER_MSVC_141)")
			set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_MSVC_141 ON)
			set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_POSTFIX "2017")

		elseif(MSVC_VERSION GREATER_EQUAL 1900)
			message(STATUS "Compiler is Visual Studio 2015 (WD_CMAKE_COMPILER_MSVC_140)")
			set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_MSVC_140 ON)
			set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_POSTFIX "2015")

		else()
			message(FATAL_ERROR "Compiler for generator '${CMAKE_GENERATOR}' is not supported on MSVC! Please extend wd_detect_compiler()")
		endif()

	elseif(WD_DETECTED_COMPILER STREQUAL "clang")
		message(STATUS "Compiler is clang (WD_CMAKE_COMPILER_CLANG)")
		set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_CLANG ON)
		set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_POSTFIX "Clang")

	elseif(WD_DETECTED_COMPILER STREQUAL "gcc")
		message(STATUS "Compiler is gcc (WD_CMAKE_COMPILER_GCC)")
		set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_GCC ON)
		set_property(GLOBAL PROPERTY WD_CMAKE_COMPILER_POSTFIX "Gcc")

	else()
		message(FATAL_ERROR "Unhandled compiler ${WD_DETECTED_COMPILER}")
	endif()

	set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_POSTFIX "")
	set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_32BIT OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_64BIT OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_X86 OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_ARM OFF)
	set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_EMSCRIPTEN OFF)

	if(WD_DETECTED_ARCH STREQUAL "x86")
		message(STATUS "Architecture is X86 (WD_CMAKE_ARCHITECTURE_X86)")
		set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_X86 ON)

		message(STATUS "Architecture is 32-Bit (WD_CMAKE_ARCHITECTURE_32BIT)")
		set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_32BIT ON)

	elseif(WD_DETECTED_ARCH STREQUAL "x64")
		message(STATUS "Architecture is X86 (WD_CMAKE_ARCHITECTURE_X86)")
		set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_X86 ON)

		message(STATUS "Architecture is 64-Bit (WD_CMAKE_ARCHITECTURE_64BIT)")
		set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_64BIT ON)

	elseif(WD_DETECTED_ARCH STREQUAL "arm32")
		message(STATUS "Architecture is ARM (WD_CMAKE_ARCHITECTURE_ARM)")
		set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_ARM ON)

		message(STATUS "Architecture is 32-Bit (WD_CMAKE_ARCHITECTURE_32BIT)")
		set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_32BIT ON)

	elseif(WD_DETECTED_ARCH STREQUAL "arm64")
		message(STATUS "Architecture is ARM (WD_CMAKE_ARCHITECTURE_ARM)")
		set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_ARM ON)

		message(STATUS "Architecture is 64-Bit (WD_CMAKE_ARCHITECTURE_64BIT)")
		set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_64BIT ON)

	elseif(WD_DETECTED_ARCH STREQUAL "emscripten")
		message(STATUS "Architecture is WEBASSEMBLY (WD_CMAKE_ARCHITECTURE_WEBASSEMBLY)")
		set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_WEBASSEMBLY ON)

		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			message(STATUS "Architecture is 64-Bit (WD_CMAKE_ARCHITECTURE_64BIT)")
			set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_64BIT ON)
		else()
			message(STATUS "Architecture is 32-Bit (WD_CMAKE_ARCHITECTURE_32BIT)")
			set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_32BIT ON)
		endif()

	else()
		message(FATAL_ERROR "Unhandled target architecture ${WD_DETECTED_ARCH}")
	endif()

	get_property(WD_CMAKE_ARCHITECTURE_32BIT GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_32BIT)
	get_property(WD_CMAKE_ARCHITECTURE_ARM GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_ARM)

	if(WD_CMAKE_ARCHITECTURE_ARM)
		if(WD_CMAKE_ARCHITECTURE_32BIT)
			set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_POSTFIX "Arm32")
		else()
			set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_POSTFIX "Arm64")
		endif()
	else()
		if(WD_CMAKE_ARCHITECTURE_32BIT)
			set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_POSTFIX "32")
		else()
			set_property(GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_POSTFIX "64")
		endif()
	endif()
endfunction()

# #####################################
# ## wd_pull_compiler_vars()
# #####################################
macro(wd_pull_compiler_and_architecture_vars)
	wd_detect_compiler_and_architecture()

	get_property(WD_CMAKE_COMPILER_POSTFIX GLOBAL PROPERTY WD_CMAKE_COMPILER_POSTFIX)
	get_property(WD_CMAKE_COMPILER_MSVC GLOBAL PROPERTY WD_CMAKE_COMPILER_MSVC)
	get_property(WD_CMAKE_COMPILER_MSVC_140 GLOBAL PROPERTY WD_CMAKE_COMPILER_MSVC_140)
	get_property(WD_CMAKE_COMPILER_MSVC_141 GLOBAL PROPERTY WD_CMAKE_COMPILER_MSVC_141)
	get_property(WD_CMAKE_COMPILER_MSVC_142 GLOBAL PROPERTY WD_CMAKE_COMPILER_MSVC_142)
	get_property(WD_CMAKE_COMPILER_MSVC_143 GLOBAL PROPERTY WD_CMAKE_COMPILER_MSVC_143)
	get_property(WD_CMAKE_COMPILER_CLANG GLOBAL PROPERTY WD_CMAKE_COMPILER_CLANG)
	get_property(WD_CMAKE_COMPILER_GCC GLOBAL PROPERTY WD_CMAKE_COMPILER_GCC)

	get_property(WD_CMAKE_ARCHITECTURE_POSTFIX GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_POSTFIX)
	get_property(WD_CMAKE_ARCHITECTURE_32BIT GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_32BIT)
	get_property(WD_CMAKE_ARCHITECTURE_64BIT GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_64BIT)
	get_property(WD_CMAKE_ARCHITECTURE_X86 GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_X86)
	get_property(WD_CMAKE_ARCHITECTURE_ARM GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_ARM)
	get_property(WD_CMAKE_ARCHITECTURE_WEBASSEMBLY GLOBAL PROPERTY WD_CMAKE_ARCHITECTURE_WEBASSEMBLY)
endmacro()

# #####################################
# ## wd_pull_all_vars()
# #####################################
macro(wd_pull_all_vars)
	get_property(WD_SUBMODULE_PREFIX_PATH GLOBAL PROPERTY WD_SUBMODULE_PREFIX_PATH)

	wd_pull_version()
	wd_pull_compiler_and_architecture_vars()
	wd_pull_generator_vars()
	wd_pull_platform_vars()
endmacro()

# #####################################
# ## wd_get_version(<VERSIONFILE> <OUT_MAJOR> <OUT_MINOR> <OUT_PATCH>)
# #####################################
function(wd_get_version VERSIONFILE OUT_MAJOR OUT_MINOR OUT_PATCH)
	file(READ ${VERSIONFILE} VERSION_STRING)

	string(STRIP ${VERSION_STRING} VERSION_STRING)

	if(VERSION_STRING MATCHES "([0-9]+).([0-9]+).([0-9+])")
		STRING(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" VERSION_MAJOR "${VERSION_STRING}")
		STRING(REGEX REPLACE "^[0-9]+\\.([0-9]+)\\.[0-9]+" "\\1" VERSION_MINOR "${VERSION_STRING}")
		STRING(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" VERSION_PATCH "${VERSION_STRING}")

		string(STRIP ${VERSION_MAJOR} VERSION_MAJOR)
		string(STRIP ${VERSION_MINOR} VERSION_MINOR)
		string(STRIP ${VERSION_PATCH} VERSION_PATCH)

		set(${OUT_MAJOR} ${VERSION_MAJOR} PARENT_SCOPE)
		set(${OUT_MINOR} ${VERSION_MINOR} PARENT_SCOPE)
		set(${OUT_PATCH} ${VERSION_PATCH} PARENT_SCOPE)

	else()
		message(FATAL_ERROR "Invalid version string '${VERSION_STRING}'")
	endif()
endfunction()

# #####################################
# ## wd_detect_version()
# #####################################
function(wd_detect_version)
	get_property(VERSION_MAJOR GLOBAL PROPERTY WD_CMAKE_SDKVERSION_MAJOR)

	if(VERSION_MAJOR)
		# has already run before and WD_CMAKE_SDKVERSION_MAJOR is already set
		return()
	endif()

	wd_get_version("${CMAKE_SOURCE_DIR}/${WD_SUBMODULE_PREFIX_PATH}/version.txt" VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

	set_property(GLOBAL PROPERTY WD_CMAKE_SDKVERSION_MAJOR "${VERSION_MAJOR}")
	set_property(GLOBAL PROPERTY WD_CMAKE_SDKVERSION_MINOR "${VERSION_MINOR}")
	set_property(GLOBAL PROPERTY WD_CMAKE_SDKVERSION_PATCH "${VERSION_PATCH}")

	message(STATUS "SDK version: Major = '${VERSION_MAJOR}', Minor = '${VERSION_MINOR}', Patch = '${VERSION_PATCH}'")
endfunction()

# #####################################
# ## wd_pull_version()
# #####################################
macro(wd_pull_version)
	wd_detect_version()

	get_property(WD_CMAKE_SDKVERSION_MAJOR GLOBAL PROPERTY WD_CMAKE_SDKVERSION_MAJOR)
	get_property(WD_CMAKE_SDKVERSION_MINOR GLOBAL PROPERTY WD_CMAKE_SDKVERSION_MINOR)
	get_property(WD_CMAKE_SDKVERSION_PATCH GLOBAL PROPERTY WD_CMAKE_SDKVERSION_PATCH)
endmacro()