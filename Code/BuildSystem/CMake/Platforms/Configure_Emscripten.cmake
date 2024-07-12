include("${CMAKE_CURRENT_LIST_DIR}/Configure_Default.cmake")

message(STATUS "Configuring Platform: Emscripten")

set_property(GLOBAL PROPERTY NS_CMAKE_PLATFORM_EMSCRIPTEN ON)

macro(ns_platform_pull_properties)

	get_property(NS_CMAKE_PLATFORM_EMSCRIPTEN GLOBAL PROPERTY NS_CMAKE_PLATFORM_EMSCRIPTEN)

endmacro()

macro(ns_platform_detect_generator)
	if(CMAKE_GENERATOR MATCHES "Ninja" OR CMAKE_GENERATOR MATCHES "Ninja Multi-Config")
		message(STATUS "Buildsystem is Ninja (NS_CMAKE_GENERATOR_NINJA)")

		set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_NINJA ON)
		set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_PREFIX "Ninja")
		set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_CONFIGURATION ${CMAKE_BUILD_TYPE})

	else()
		message(FATAL_ERROR "Generator '${CMAKE_GENERATOR}' is not supported on Emscripten! Please extend ns_platform_detect_generator()")
	endif()
endmacro()