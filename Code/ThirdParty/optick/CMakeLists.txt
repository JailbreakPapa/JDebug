wd_cmake_init()

wd_requires_vulkan()

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

wd_create_target(SHARED_LIBRARY ${PROJECT_NAME} NO_WD_PREFIX THIRD_PARTY)


if(WD_CMAKE_COMPILER_MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4456)
endif()

wd_link_target_vulkan(${PROJECT_NAME})

set(WD_OPTICK_PROFILER_SUPPORT ON CACHE BOOL "Support for profiling with Optick.")
