
if(CMAKE_SYSTEM_NAME STREQUAL "Prospero") # PlayStation 5

    set_property(GLOBAL PROPERTY NS_CMAKE_PLATFORM_NAME "Playstation5")
    set_property(GLOBAL PROPERTY NS_CMAKE_GENERATOR_PREFIX "PS5")
    set_property(GLOBAL PROPERTY NS_CMAKE_COMPILER_POSTFIX "prosperoclang")
    set_property(GLOBAL PROPERTY NS_CMAKE_PLATFORM_PREFIX "Playstation")
    set_property(GLOBAL PROPERTY NS_CMAKE_PLATFORM_POSTFIX "Playstation")
endif()