# This is a NDA Protected Library. The Developers GPDA should already be signed with SIE Partners.
# include(${CMAKE_SOURCE_DIR}/Code/BuildSystem/CMake/Internal/NorthstarPS5.cmake)
# Checks if the client is a developer, if the PS Developer Key is installed on the computer.

include("${CMAKE_CURRENT_LIST_DIR}/Configure_Default.cmake")

message(STATUS "Configuring NDA Platform: Prospero (Playstation 5)")

set_property(GLOBAL PROPERTY NS_CMAKE_PLATFORM_PLAYSTATION_5 ON)
set_property(GLOBAL PROPERTY NS_CMAKE_PLATFORM_CONSOLE ON)
set_property(GLOBAL PROPERTY NS_CMAKE_PLATFORM_SUPPORTS_VULKAN OFF)
set_property(GLOBAL PROPERTY NS_CMAKE_PLATFORM_SUPPORTS_EDITOR OFF)
set_property(GLOBAL PROPERTY NS_CMAKE_PLATFORM_POSIX ON)



# #####################################
# ## General settings
# #####################################
set(NS_COMPILE_ENGINE_AS_DLL ON CACHE BOOL "Whether to compile the code as a shared libraries (DLL/PRX).")
mark_as_advanced(FORCE NS_COMPILE_ENGINE_AS_DLL)

macro(ns_platform_pull_properties)
    get_property(NS_CONFIG_PLAYSTATION_ALLOWED GLOBAL PROPERTY NS_CONFIG_PLAYSTATION_ALLOWED)
    get_property(NS_CMAKE_PLATFORM_CONSOLE GLOBAL PROPERTY NS_CMAKE_PLATFORM_CONSOLE)
    get_property(NS_CMAKE_PLATFORM_PLAYSTATION_5 GLOBAL PROPERTY NS_CMAKE_PLATFORM_PLAYSTATION_5)
    get_property(NS_CMAKE_PLATFORM_SUPPORTS_VULKAN GLOBAL PROPERTY NS_CMAKE_PLATFORM_SUPPORTS_VULKAN)
    get_property(NS_CMAKE_PLATFORM_SUPPORTS_EDITOR GLOBAL PROPERTY NS_CMAKE_PLATFORM_SUPPORTS_EDITOR)
endmacro()

macro(ns_platformhook_set_build_flags_clang)
    # Disable the warning that clang doesn't support pragma optimize.
    target_compile_options(${TARGET_NAME} PRIVATE -Wno-ignored-pragma-optimize -Wno-pragma-pack)
endmacro()

macro(ns_platform_detect_generator)
    string(FIND ${CMAKE_VERSION} "MSVC" VERSION_CONTAINS_MSVC)

    if(${VERSION_CONTAINS_MSVC} GREATER -1)
        message(STATUS "CMake was called from Visual Studio Open Folder workflow")
        set_property(GLOBAL PROPERTY NS_CMAKE_INSIDE_VS ON)
    endif()

    if(NS_CMAKE_PLATFORM_PLAYSTATION_5 OR CMAKE_GENERATOR_PLATFORM STREQUAL "Prospero") # NOTE: PS5 should ONLY use its compiler.
        if(CMAKE_GENERATOR MATCHES "Visual Studio")
            add_link_options("$<$<OR:$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>,$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>>:--prx-stub-output-dir$<TARGET_LINKER_FILE_DIR:$<TARGET_PROPERTY:NAME>>>")

            # set(CMAKE_GENERATOR_TOOLSET "PS5Clang")

            # NOTE: NorthstarPS5 should have set all required vars, etc.. so lets just notify the user.
            message(STATUS "Generator is PS5Clang (NS_CMAKE_GENERATOR_CLANG_PROSPERO)")
            set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_CLANG_PROSPERO ON)
            
            set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_CONFIGURATION $<CONFIGURATION>)
        elseif(CMAKE_GENERATOR MATCHES "Ninja")
            set(CMAKE_C_CREATE_SHARED_LIBRARY "<CMAKE_LINKER> cpplink -o <TARGET> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES>" CACHE STRING "" FORCE)
            set(CMAKE_CXX_CREATE_SHARED_LIBRARY "<CMAKE_LINKER> cpplink -o <TARGET> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES>" CACHE STRING "" FORCE)
            string(CONCAT CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --oformat=prx ${LINKER_FLAGS}")
            set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_CONFIGURATION ${CMAKE_BUILD_TYPE})
            enable_language(PS5PSSL)
        endif()
    else()
        message(FATAL_ERROR "Generator '${CMAKE_GENERATOR}' is not supported on Prospero! Please extend ns_platform_detect_generator() !")
    endif()
endmacro()