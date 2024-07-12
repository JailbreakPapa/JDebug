#pragma once

#ifdef BUILDSYSTEM_ENABLE_GLFW_SUPPORT
#  define NS_SUPPORTS_GLFW NS_ON
#else
#  define NS_SUPPORTS_GLFW NS_OFF
#endif

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  include <Foundation/Basics/Platform/Win/PlatformFeatures_win.h>
#elif NS_ENABLED(NS_PLATFORM_OSX)
#  include <Foundation/Basics/Platform/OSX/PlatformFeatures_OSX.h>
#elif NS_ENABLED(NS_PLATFORM_LINUX)
#  include <Foundation/Basics/Platform/Linux/PlatformFeatures_Linux.h>
#elif NS_ENABLED(NS_PLATFORM_ANDROID)
#  include <Foundation/Basics/Platform/Android/PlatformFeatures_Android.h>
#elif NS_ENABLED(NS_PLATFORM_PLAYSTATION_5)
#  include <Foundation/Basics/Platform/Prospero/PlatformFeatures_Prospero.h>
#else
#  error "Undefined platform!"
#endif

// now check that the defines for each feature are set (either to 1 or 0, but they must be defined)

#ifndef NS_SUPPORTS_FILE_ITERATORS
#  error "NS_SUPPORTS_FILE_ITERATORS is not defined."
#endif

#ifndef NS_USE_POSIX_FILE_API
#  error "NS_USE_POSIX_FILE_API is not defined."
#endif

#ifndef NS_SUPPORTS_FILE_STATS
#  error "NS_SUPPORTS_FILE_STATS is not defined."
#endif

#ifndef NS_SUPPORTS_MEMORY_MAPPED_FILE
#  error "NS_SUPPORTS_MEMORY_MAPPED_FILE is not defined."
#endif

#ifndef NS_SUPPORTS_SHARED_MEMORY
#  error "NS_SUPPORTS_SHARED_MEMORY is not defined."
#endif

#ifndef NS_SUPPORTS_DYNAMIC_PLUGINS
#  error "NS_SUPPORTS_DYNAMIC_PLUGINS is not defined."
#endif

#ifndef NS_SUPPORTS_UNRESTRICTED_FILE_ACCESS
#  error "NS_SUPPORTS_UNRESTRICTED_FILE_ACCESS is not defined."
#endif

#ifndef NS_SUPPORTS_CASE_INSENSITIVE_PATHS
#  error "NS_SUPPORTS_CASE_INSENSITIVE_PATHS is not defined."
#endif

#ifndef NS_SUPPORTS_LONG_PATHS
#  error "NS_SUPPORTS_LONG_PATHS is not defined."
#endif
