# #####################################
# ## ns_detect_project_name(<out-name>)
# #####################################

function(ns_detect_project_name OUT_NAME)
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
# ## ns_pull_platform_vars()
# #####################################
macro(ns_pull_platform_vars)

	get_property(NS_CMAKE_PLATFORM_NAME GLOBAL PROPERTY NS_CMAKE_PLATFORM_NAME)
	get_property(NS_CMAKE_PLATFORM_PREFIX GLOBAL PROPERTY NS_CMAKE_PLATFORM_PREFIX)
	get_property(NS_CMAKE_PLATFORM_POSTFIX GLOBAL PROPERTY NS_CMAKE_PLATFORM_POSTFIX)
	get_property(NS_CMAKE_PLATFORM_POSIX GLOBAL PROPERTY NS_CMAKE_PLATFORM_POSIX)
	get_property(NS_CMAKE_PLATFORM_SUPPORTS_VULKAN GLOBAL PROPERTY NS_CMAKE_PLATFORM_SUPPORTS_VULKAN)
	get_property(NS_CMAKE_PLATFORM_SUPPORTS_EDITOR GLOBAL PROPERTY NS_CMAKE_PLATFORM_SUPPORTS_EDITOR)
	get_property(NS_CMAKE_PLATFORM_CONSOLE GLOBAL PROPERTY NS_CMAKE_PLATFORM_CONSOLE)

	ns_platform_pull_properties()
endmacro()

# #####################################
# ## ns_detect_generator()
# #####################################
function(ns_detect_generator)
	get_property(PREFIX GLOBAL PROPERTY NS_CMAKE_GENERATOR_PREFIX)

	if(PREFIX)
		# has already run before and NS_CMAKE_GENERATOR_PREFIX is already set
		# message (STATUS "Redundant call to ns_detect_generator()")
		return()
	endif()

	ns_pull_platform_vars()

	set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_PREFIX "")
	set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_CONFIGURATION "undefined")
	set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_MSVC OFF)
	set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_XCODE OFF)
	set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_MAKE OFF)
	set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_NINJA OFF)
	set_property(GLOBAL PROPERTY NS_CMAKE_INSIDE_VS OFF) # if cmake is called through the visual studio open folder workflow
	set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_CLANG_PLAYSTATION OFF)

	message(STATUS "CMAKE_VERSION is '${CMAKE_VERSION}'")
	message(STATUS "CMAKE_BUILD_TYPE is '${CMAKE_BUILD_TYPE}'")
	message(STATUS "CMAKE_GENERATOR is '${CMAKE_GENERATOR}'")

	ns_platform_detect_generator()

endfunction()

# #####################################
# ## ns_pull_generator_vars()
# #####################################
macro(ns_pull_generator_vars)
	ns_detect_generator()

	get_property(NS_CMAKE_GENERATOR_PREFIX GLOBAL PROPERTY NS_CMAKE_GENERATOR_PREFIX)
	get_property(NS_CMAKE_GENERATOR_CONFIGURATION GLOBAL PROPERTY NS_CMAKE_GENERATOR_CONFIGURATION)
	get_property(NS_CMAKE_GENERATOR_CLANG_PLAYSTATION GLOBAL PROPERTY NS_CMAKE_GENERATOR_CLANG_PLAYSTATION)
	get_property(NS_CMAKE_GENERATOR_MSVC GLOBAL PROPERTY NS_CMAKE_GENERATOR_MSVC)
	get_property(NS_CMAKE_GENERATOR_XCODE GLOBAL PROPERTY NS_CMAKE_GENERATOR_XCODE)
	get_property(NS_CMAKE_GENERATOR_MAKE GLOBAL PROPERTY NS_CMAKE_GENERATOR_MAKE)
	get_property(NS_CMAKE_GENERATOR_NINJA GLOBAL PROPERTY NS_CMAKE_GENERATOR_NINJA)
	get_property(NS_CMAKE_INSIDE_VS GLOBAL PROPERTY NS_CMAKE_INSIDE_VS)
endmacro()

# #####################################
# ## ns_detect_compiler_and_architecture()
# #####################################
function(ns_detect_compiler_and_architecture)
	get_property(PREFIX GLOBAL PROPERTY NS_CMAKE_COMPILER_POSTFIX)

	if(PREFIX)
		# has already run before and NS_CMAKE_COMPILER_POSTFIX is already set
		# message (STATUS "Redundant call to ns_detect_compiler()")
		return()
	endif()

	ns_pull_platform_vars()
	ns_pull_generator_vars()
	ns_pull_config_vars()
	get_property(GENERATOR_MSVC GLOBAL PROPERTY NS_CMAKE_GENERATOR_MSVC)

	set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_POSTFIX "")
	set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_MSVC OFF)
	set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_MSVC_140 OFF)
	set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_MSVC_141 OFF)
	set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_MSVC_142 OFF)
	set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_MSVC_143 OFF)
	set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_CLANG OFF)
	set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_GCC OFF)

	set(FILE_TO_COMPILE "${NS_ROOT}/${NS_CMAKE_RELPATH}/ProbingSrc/ArchitectureDetect.c")
	
	if (NS_SDK_DIR)
		set(FILE_TO_COMPILE "${NS_SDK_DIR}/${NS_CMAKE_RELPATH}/ProbingSrc/ArchitectureDetect.c")
	endif()

	# Only compile the detect file if we don't have a cached result from the last run
	if((NOT NS_DETECTED_COMPILER) OR (NOT NS_DETECTED_ARCH) OR (NOT NS_DETECTED_MSVC_VER))
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
			set(NS_DETECTED_ARCH ${CMAKE_MATCH_1} CACHE INTERNAL "")
		else()
			message(FATAL_ERROR "The compile test did not output the architecture. Compiler broken? Compiler output: ${COMPILE_OUTPUT}")
		endif()

		if(${COMPILE_OUTPUT} MATCHES "COMPILER:'([^']*)'")
			set(NS_DETECTED_COMPILER ${CMAKE_MATCH_1} CACHE INTERNAL "")
		else()
			message(FATAL_ERROR "The compile test did not output the compiler. Compiler broken? Compiler output: ${COMPILE_OUTPUT}")
		endif()
		
		if(NS_DETECTED_COMPILER STREQUAL "msvc")
			if(${COMPILE_OUTPUT} MATCHES "MSC_VER:'([^']*)'")
				set(NS_DETECTED_MSVC_VER ${CMAKE_MATCH_1} CACHE INTERNAL "")
			else()
				message(FATAL_ERROR "The compile test did not output the MSC_VER. Compiler broken? Compiler output: ${COMPILE_OUTPUT}")
			endif()
		else()
			set(NS_DETECTED_MSVC_VER "<NOT USING MSVC>" CACHE INTERNAL "")
		endif()
	endif()

	if(NS_DETECTED_COMPILER STREQUAL "msvc") # Visual Studio Compiler
		message(STATUS "Compiler is MSVC (NS_CMAKE_COMPILER_MSVC) version ${NS_DETECTED_MSVC_VER}")

		set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_MSVC ON)

		if(NS_DETECTED_MSVC_VER GREATER_EQUAL 1930)
			message(STATUS "Compiler is Visual Studio 2022 (NS_CMAKE_COMPILER_MSVC_143)")
			set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_MSVC_143 ON)
			set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_POSTFIX "2022")

		elseif(NS_DETECTED_MSVC_VER GREATER_EQUAL 1920)
			message(STATUS "Compiler is Visual Studio 2019 (NS_CMAKE_COMPILER_MSVC_142)")
			set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_MSVC_142 ON)
			set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_POSTFIX "2019")

		elseif(NS_DETECTED_MSVC_VER GREATER_EQUAL 1910)
			message(STATUS "Compiler is Visual Studio 2017 (NS_CMAKE_COMPILER_MSVC_141)")
			set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_MSVC_141 ON)
			set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_POSTFIX "2017")

		elseif(MSVC_VERSION GREATER_EQUAL 1900)
			message(STATUS "Compiler is Visual Studio 2015 (NS_CMAKE_COMPILER_MSVC_140)")
			set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_MSVC_140 ON)
			set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_POSTFIX "2015")

		else()
			message(FATAL_ERROR "Compiler for generator '${CMAKE_GENERATOR}' is not supported on MSVC! Please extend ns_detect_compiler()")
		endif()

	elseif(NS_DETECTED_COMPILER STREQUAL "clang")
		message(STATUS "Compiler is clang (NS_CMAKE_COMPILER_CLANG)")
		set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_CLANG ON)
		set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_POSTFIX "Clang")

	elseif(NS_DETECTED_COMPILER STREQUAL "gcc")
		message(STATUS "Compiler is gcc (NS_CMAKE_COMPILER_GCC)")
		set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_GCC ON)
		set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_POSTFIX "Gcc")
		
	else()
		message(FATAL_ERROR "Unhandled compiler ${NS_DETECTED_COMPILER}")
	endif()

	set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_POSTFIX "")
	set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_32BIT OFF)
	set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_64BIT OFF)
	set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_X86 OFF)
	set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_ARM OFF)
	set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_EMSCRIPTEN OFF)
	if(NS_DETECTED_ARCH STREQUAL "x86")
		message(STATUS "Architecture is X86 (NS_CMAKE_ARCHITECTURE_X86)")
		set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_X86 ON)

		message(STATUS "Architecture is 32-Bit (NS_CMAKE_ARCHITECTURE_32BIT)")
		set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_32BIT ON)

	elseif(NS_DETECTED_ARCH STREQUAL "x64")
		message(STATUS "Architecture is X86 (NS_CMAKE_ARCHITECTURE_X86)")
		set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_X86 ON)

		message(STATUS "Architecture is 64-Bit (NS_CMAKE_ARCHITECTURE_64BIT)")
		set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_64BIT ON)

	elseif(NS_DETECTED_ARCH STREQUAL "arm32")
		message(STATUS "Architecture is ARM (NS_CMAKE_ARCHITECTURE_ARM)")
		set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_ARM ON)

		message(STATUS "Architecture is 32-Bit (NS_CMAKE_ARCHITECTURE_32BIT)")
		set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_32BIT ON)

	elseif(NS_DETECTED_ARCH STREQUAL "arm64")
		message(STATUS "Architecture is ARM (NS_CMAKE_ARCHITECTURE_ARM)")
		set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_ARM ON)

		message(STATUS "Architecture is 64-Bit (NS_CMAKE_ARCHITECTURE_64BIT)")
		set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_64BIT ON)
	elseif(NS_DETECTED_ARCH STREQUAL "emscripten")
		message(STATUS "Architecture is WEBASSEMBLY (NS_CMAKE_ARCHITECTURE_WEBASSEMBLY)")
		set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_WEBASSEMBLY ON)

		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			message(STATUS "Architecture is 64-Bit (NS_CMAKE_ARCHITECTURE_64BIT)")
			set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_64BIT ON)
		else()
			message(STATUS "Architecture is 32-Bit (NS_CMAKE_ARCHITECTURE_32BIT)")
			set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_32BIT ON)
		endif()
	
	else()
		message(FATAL_ERROR "Unhandled target architecture ${NS_DETECTED_ARCH}")
	endif()

	get_property(NS_CMAKE_ARCHITECTURE_32BIT GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_32BIT)
	get_property(NS_CMAKE_ARCHITECTURE_ARM GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_ARM)

	if(NS_CMAKE_ARCHITECTURE_ARM)
		if(NS_CMAKE_ARCHITECTURE_32BIT)
			set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_POSTFIX "Arm32")
		else()
			set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_POSTFIX "Arm64")
		endif()
	else()
		if(NS_CMAKE_ARCHITECTURE_32BIT)
			set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_POSTFIX "32")
		else()
			set_property(GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_POSTFIX "64")
		endif()
	endif()
endfunction()

# #####################################
# ## ns_pull_compiler_vars()
# #####################################
macro(ns_pull_compiler_and_architecture_vars)
	ns_detect_compiler_and_architecture()

	get_property(NS_CMAKE_COMPILER_POSTFIX GLOBAL PROPERTY NS_CMAKE_COMPILER_POSTFIX)
	get_property(NS_CMAKE_COMPILER_MSVC GLOBAL PROPERTY NS_CMAKE_COMPILER_MSVC)
	get_property(NS_CMAKE_COMPILER_MSVC_140 GLOBAL PROPERTY NS_CMAKE_COMPILER_MSVC_140)
	get_property(NS_CMAKE_COMPILER_MSVC_141 GLOBAL PROPERTY NS_CMAKE_COMPILER_MSVC_141)
	get_property(NS_CMAKE_COMPILER_MSVC_142 GLOBAL PROPERTY NS_CMAKE_COMPILER_MSVC_142)
	get_property(NS_CMAKE_COMPILER_MSVC_143 GLOBAL PROPERTY NS_CMAKE_COMPILER_MSVC_143)
	get_property(NS_CMAKE_COMPILER_CLANG GLOBAL PROPERTY NS_CMAKE_COMPILER_CLANG)
	get_property(NS_CMAKE_COMPILER_GCC GLOBAL PROPERTY NS_CMAKE_COMPILER_GCC)


	get_property(NS_CMAKE_ARCHITECTURE_POSTFIX GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_POSTFIX)
	get_property(NS_CMAKE_ARCHITECTURE_32BIT GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_32BIT)
	get_property(NS_CMAKE_ARCHITECTURE_64BIT GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_64BIT)
	get_property(NS_CMAKE_ARCHITECTURE_X86 GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_X86)
	get_property(NS_CMAKE_ARCHITECTURE_ARM GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_ARM)
	get_property(NS_CMAKE_ARCHITECTURE_WEBASSEMBLY GLOBAL PROPERTY NS_CMAKE_ARCHITECTURE_WEBASSEMBLY)
endmacro()

# #####################################
# ## ns_pull_all_vars()
# #####################################
macro(ns_pull_all_vars)
	get_property(NS_SUBMODULE_PREFIX_PATH GLOBAL PROPERTY NS_SUBMODULE_PREFIX_PATH)
	get_property(NS_ROOT GLOBAL PROPERTY NS_ROOT)

	ns_pull_version()
	ns_pull_compiler_and_architecture_vars()
	ns_pull_generator_vars()
	ns_pull_platform_vars()
endmacro()

# #####################################
# ## ns_get_version(<VERSIONFILE> <OUT_MAJOR> <OUT_MINOR> <OUT_PATCH>)
# #####################################
function(ns_get_version VERSIONFILE OUT_MAJOR OUT_MINOR OUT_PATCH)
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
# ## ns_detect_version()
# #####################################
function(ns_detect_version)
	get_property(VERSION_MAJOR GLOBAL PROPERTY NS_CMAKE_SDKVERSION_MAJOR)

	if(VERSION_MAJOR)
		# has already run before and NS_CMAKE_SDKVERSION_MAJOR is already set
		return()
	endif()

	ns_get_version("${NS_ROOT}/version.txt" VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

	set_property(GLOBAL PROPERTY NS_CMAKE_SDKVERSION_MAJOR "${VERSION_MAJOR}")
	set_property(GLOBAL PROPERTY NS_CMAKE_SDKVERSION_MINOR "${VERSION_MINOR}")
	set_property(GLOBAL PROPERTY NS_CMAKE_SDKVERSION_PATCH "${VERSION_PATCH}")

	message(STATUS "SDK version: Major = '${VERSION_MAJOR}', Minor = '${VERSION_MINOR}', Patch = '${VERSION_PATCH}'")
endfunction()

# #####################################
# ## ns_pull_version()
# #####################################
macro(ns_pull_version)
	ns_detect_version()

	get_property(NS_CMAKE_SDKVERSION_MAJOR GLOBAL PROPERTY NS_CMAKE_SDKVERSION_MAJOR)
	get_property(NS_CMAKE_SDKVERSION_MINOR GLOBAL PROPERTY NS_CMAKE_SDKVERSION_MINOR)
	get_property(NS_CMAKE_SDKVERSION_PATCH GLOBAL PROPERTY NS_CMAKE_SDKVERSION_PATCH)
endmacro()