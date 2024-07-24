#pragma once

/// \file

/// \brief Used in conjunction with NS_ENABLED and NS_DISABLED for safe checks. Define something to NS_ON or NS_OFF to work with those macros.
#define NS_ON =

/// \brief Used in conjunction with NS_ENABLED and NS_DISABLED for safe checks. Define something to NS_ON or NS_OFF to work with those macros.
#define NS_OFF !

/// \brief Used in conjunction with NS_ON and NS_OFF for safe checks. Use #if NS_ENABLED(x) or #if NS_DISABLED(x) in conditional compilation.
#define NS_ENABLED(x) (1 NS_CONCAT(x, =) 1)

/// \brief Used in conjunction with NS_ON and NS_OFF for safe checks. Use #if NS_ENABLED(x) or #if NS_DISABLED(x) in conditional compilation.
#define NS_DISABLED(x) (1 NS_CONCAT(x, =) 2)

/// \brief Checks whether x AND y are both defined as NS_ON or NS_OFF. Usually used to check whether configurations overlap, to issue an error.
#define NS_IS_NOT_EXCLUSIVE(x, y) ((1 NS_CONCAT(x, =) 1) == (1 NS_CONCAT(y, =) 1))



// All the supported Platforms
#define NS_PLATFORM_WINDOWS NS_OFF         // enabled for all Windows platforms, both UWP and desktop
#define NS_PLATFORM_WINDOWS_UWP NS_OFF     // enabled for UWP apps, together with NS_PLATFORM_WINDOWS
#define NS_PLATFORM_WINDOWS_DESKTOP NS_OFF // enabled for desktop apps, together with NS_PLATFORM_WINDOWS
#define NS_PLATFORM_OSX NS_OFF
#define NS_PLATFORM_LINUX NS_OFF
#define NS_PLATFORM_IOS NS_OFF
#define NS_PLATFORM_ANDROID NS_OFF
#define NS_PLATFORM_PLAYSTATION_5 NS_OFF
#define NS_PLATFORM_XBOX_NS_OFF
#define NS_PLATFORM_SWITCH NS_OFF
#define NS_PLATFORM_CONSOLE NS_OFF
// Different Bit OSes
#define NS_PLATFORM_32BIT NS_OFF
#define NS_PLATFORM_64BIT NS_OFF

// Different CPU architectures
#define NS_PLATFORM_ARCH_X86 NS_OFF
#define NS_PLATFORM_ARCH_ARM NS_OFF

// Endianess
#define NS_PLATFORM_LITTLE_ENDIAN NS_OFF
#define NS_PLATFORM_BIG_ENDIAN NS_OFF

// Different Compilers
#define NS_COMPILER_MSVC NS_OFF
#define NS_COMPILER_MSVC_CLANG NS_OFF // Clang front-end with MSVC CodeGen
#define NS_COMPILER_MSVC_PURE NS_OFF  // MSVC front-end and CodeGen, no mixed compilers
#define NS_COMPILER_CLANG NS_OFF
#define NS_COMPILER_GCC NS_OFF

// How to compile the engine
#define NS_COMPILE_ENGINE_AS_DLL NS_OFF
#define NS_COMPILE_FOR_DEBUG NS_OFF
#define NS_COMPILE_FOR_DEVELOPMENT NS_OFF

// Platform Features
#define NS_USE_POSIX_FILE_API NS_OFF
#define NS_USE_LINUX_POSIX_EXTENSIONS NS_OFF // linux specific posix extensions like pipe2, dup3, etc.
#define NS_USE_CPP20_OPERATORS NS_OFF
#define NS_SUPPORTS_FILE_ITERATORS NS_OFF
#define NS_SUPPORTS_FILE_STATS NS_OFF
#define NS_SUPPORTS_DIRECTORY_WATCHER NS_OFF
#define NS_SUPPORTS_MEMORY_MAPPED_FILE NS_OFF
#define NS_SUPPORTS_SHARED_MEMORY NS_OFF
#define NS_SUPPORTS_DYNAMIC_PLUGINS NS_OFF
#define NS_SUPPORTS_UNRESTRICTED_FILE_ACCESS NS_OFF
#define NS_SUPPORTS_CASE_INSENSITIVE_PATHS NS_OFF
#define NS_SUPPORTS_CRASH_DUMPS NS_OFF
#define NS_SUPPORTS_LONG_PATHS NS_OFF

// Allocators
#define NS_ALLOC_GUARD_ALLOCATIONS NS_OFF
#define NS_ALLOC_TRACKING_DEFAULT nsAllocatorTrackingMode::Nothing

// Other Features
#define NS_USE_PROFILING NS_OFF
#define NS_USE_STRING_VALIDATION NS_OFF

// Hashed String
/// \brief Ref counting on hashed strings adds the possibility to cleanup unused strings. Since ref counting has a performance overhead it is disabled
/// by default.
#define NS_HASHED_STRING_REF_COUNTING NS_OFF

// Math Debug Checks
#define NS_MATH_CHECK_FOR_NAN NS_OFF

// SIMD support
#define NS_SIMD_IMPLEMENTATION_FPU 1
#define NS_SIMD_IMPLEMENTATION_SSE 2
#define NS_SIMD_IMPLEMENTATION_NEON 3

#define NS_SIMD_IMPLEMENTATION 0

// Application entry point code injection (undef and redefine in UserConfig.h if needed)
#define NS_APPLICATION_ENTRY_POINT_CODE_INJECTION

// Whether 'RuntimeConfigs' files should be searched in the old location
#define NS_MIGRATE_RUNTIMECONFIGS NS_OFF

// Interoperability with other libraries
#define NS_INTEROP_STL_STRINGS NS_OFF
#define NS_INTEROP_STL_SPAN NS_OFF
