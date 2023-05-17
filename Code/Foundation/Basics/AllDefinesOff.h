#pragma once

/// \file

/// \brief Used in conjunction with WD_ENABLED and WD_DISABLED for safe checks. Define something to WD_ON or WD_OFF to work with those macros.
#define WD_ON =

/// \brief Used in conjunction with WD_ENABLED and WD_DISABLED for safe checks. Define something to WD_ON or WD_OFF to work with those macros.
#define WD_OFF !

/// \brief Used in conjunction with WD_ON and WD_OFF for safe checks. Use #if WD_ENABLED(x) or #if WD_DISABLED(x) in conditional compilation.
#define WD_ENABLED(x) (1 WD_CONCAT(x, =) 1)

/// \brief Used in conjunction with WD_ON and WD_OFF for safe checks. Use #if WD_ENABLED(x) or #if WD_DISABLED(x) in conditional compilation.
#define WD_DISABLED(x) (1 WD_CONCAT(x, =) 2)

/// \brief Checks whether x AND y are both defined as WD_ON or WD_OFF. Usually used to check whether configurations overlap, to issue an error.
#define WD_IS_NOT_EXCLUSIVE(x, y) ((1 WD_CONCAT(x, =) 1) == (1 WD_CONCAT(y, =) 1))



// All the supported Platforms
#define WD_PLATFORM_WINDOWS WD_OFF         // enabled for all Windows platforms, both UWP and desktop
#define WD_PLATFORM_WINDOWS_UWP WD_OFF     // enabled for UWP apps, together with WD_PLATFORM_WINDOWS
#define WD_PLATFORM_WINDOWS_DESKTOP WD_OFF // enabled for desktop apps, together with WD_PLATFORM_WINDOWS
#define WD_PLATFORM_OSX WD_OFF
#define WD_PLATFORM_LINUX WD_OFF
#define WD_PLATFORM_IOS WD_OFF
#define WD_PLATFORM_ANDROID WD_OFF

// Different Bit OSes
#define WD_PLATFORM_32BIT WD_OFF
#define WD_PLATFORM_64BIT WD_OFF

// Different CPU architectures
#define WD_PLATFORM_ARCH_X86 WD_OFF
#define WD_PLATFORM_ARCH_ARM WD_OFF

// Endianess
#define WD_PLATFORM_LITTLE_ENDIAN WD_OFF
#define WD_PLATFORM_BIG_ENDIAN WD_OFF

// Different Compilers
#define WD_COMPILER_MSVC WD_OFF
#define WD_COMPILER_MSVC_CLANG WD_OFF // Clang front-end with MSVC CodeGen
#define WD_COMPILER_MSVC_PURE WD_OFF  // MSVC front-end and CodeGen, no mixed compilers
#define WD_COMPILER_CLANG WD_OFF
#define WD_COMPILER_GCC WD_OFF

// How to compile the engine
#define WD_COMPILE_ENGINE_AS_DLL WD_OFF
#define WD_COMPILE_FOR_DEBUG WD_OFF
#define WD_COMPILE_FOR_DEVELOPMENT WD_OFF

// Platform Features
#define WD_USE_POSIX_FILE_API WD_OFF
#define WD_SUPPORTS_FILE_ITERATORS WD_OFF
#define WD_SUPPORTS_FILE_STATS WD_OFF
#define WD_SUPPORTS_DIRECTORY_WATCHER WD_OFF
#define WD_SUPPORTS_MEMORY_MAPPED_FILE WD_OFF
#define WD_SUPPORTS_SHARED_MEMORY WD_OFF
#define WD_SUPPORTS_DYNAMIC_PLUGINS WD_OFF
#define WD_SUPPORTS_UNRESTRICTED_FILE_ACCESS WD_OFF
#define WD_SUPPORTS_CASE_INSENSITIVE_PATHS WD_OFF
#define WD_SUPPORTS_CRASH_DUMPS WD_OFF
#define WD_SUPPORTS_LONG_PATHS WD_OFF
#define WD_SUPPORTS_GLFW WD_OFF

// Allocators
#define WD_USE_ALLOCATION_TRACKING WD_OFF
#define WD_USE_ALLOCATION_STACK_TRACING WD_OFF
#define WD_USE_GUARDED_ALLOCATIONS WD_OFF

// Other Features
#define WD_USE_PROFILING WD_OFF

// Hashed String
/// \brief Ref counting on hashed strings adds the possibility to cleanup unused strings. Since ref counting has a performance overhead it is disabled
/// by default.
#define WD_HASHED_STRING_REF_COUNTING WD_OFF

// Math Debug Checks
#define WD_MATH_CHECK_FOR_NAN WD_OFF

// SIMD support
#define WD_SIMD_IMPLEMENTATION_FPU 1
#define WD_SIMD_IMPLEMENTATION_SSE 2

#define WD_SIMD_IMPLEMENTATION 0

// Application entry point code injection (undef and redefine in UserConfig.h if needed)
#define WD_APPLICATION_ENTRY_POINT_CODE_INJECTION

// Whether 'RuntimeConfigs' files should be searched in the old location
#define WD_MIGRATE_RUNTIMECONFIGS WD_OFF
