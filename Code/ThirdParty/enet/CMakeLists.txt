ns_cmake_init()

ns_requires(NS_3RDPARTY_ENET_SUPPORT)

# Get the name of this folder as the project name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)

ns_create_target(LIBRARY ${PROJECT_NAME} NO_NS_PREFIX ALL_SYMBOLS_VISIBLE)

target_compile_definitions(${PROJECT_NAME} PUBLIC BUILDSYSTEM_ENABLE_ENET_SUPPORT)

if(NS_CMAKE_PLATFORM_WINDOWS)
  target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32 _WINSOCK_DEPRECATED_NO_WARNINGS)

  if(NOT NS_CMAKE_PLATFORM_WINDOWS_UWP)

    target_link_libraries(${PROJECT_NAME}
      PRIVATE
      ws2_32.lib
      Kernel32.lib
      runtimeobject.lib
	    winmm.lib
    )

  endif()
  
  if(NS_CMAKE_COMPILER_MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4232 /wd4206 /wd4701)
  endif()
endif()

