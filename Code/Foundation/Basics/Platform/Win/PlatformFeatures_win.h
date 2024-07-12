#pragma once

/// \file

/// If set to 1, the POSIX file implementation will be used. Otherwise a platform specific implementation must be available.
#undef NS_USE_POSIX_FILE_API

#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
#  define NS_USE_POSIX_FILE_API NS_ON
#else
#  define NS_USE_POSIX_FILE_API NS_OFF
#endif

/// Iterating through the file system is supported
#undef NS_SUPPORTS_FILE_ITERATORS

#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
#  define NS_SUPPORTS_FILE_ITERATORS NS_OFF
#else
#  define NS_SUPPORTS_FILE_ITERATORS NS_ON
#endif

/// Getting the stats of a file (modification times etc.) is supported.
#undef NS_SUPPORTS_FILE_STATS
#define NS_SUPPORTS_FILE_STATS NS_ON

/// Directory watcher is supported on non uwp platforms.
#undef NS_SUPPORTS_DIRECTORY_WATCHER
#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
#  define NS_SUPPORTS_DIRECTORY_WATCHER NS_OFF
#else
#  define NS_SUPPORTS_DIRECTORY_WATCHER NS_ON
#endif

/// Memory mapping a file is supported.
#undef NS_SUPPORTS_MEMORY_MAPPED_FILE
#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
#  define NS_SUPPORTS_MEMORY_MAPPED_FILE NS_OFF
#else
#  define NS_SUPPORTS_MEMORY_MAPPED_FILE NS_ON
#endif

/// Shared memory IPC is supported.
#undef NS_SUPPORTS_SHARED_MEMORY
#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
#  define NS_SUPPORTS_SHARED_MEMORY NS_OFF
#else
#  define NS_SUPPORTS_SHARED_MEMORY NS_ON
#endif

/// Whether dynamic plugins (through DLLs loaded/unloaded at runtime) are supported
#undef NS_SUPPORTS_DYNAMIC_PLUGINS
#define NS_SUPPORTS_DYNAMIC_PLUGINS NS_ON

/// Whether applications can access any file (not sandboxed)
#undef NS_SUPPORTS_UNRESTRICTED_FILE_ACCESS
#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
#  define NS_SUPPORTS_UNRESTRICTED_FILE_ACCESS NS_OFF
#else
#  define NS_SUPPORTS_UNRESTRICTED_FILE_ACCESS NS_ON
#endif

/// Whether file accesses can be done through paths that do not match exact casing
#undef NS_SUPPORTS_CASE_INSENSITIVE_PATHS
#define NS_SUPPORTS_CASE_INSENSITIVE_PATHS NS_ON

/// Whether starting other processes is supported.
#undef NS_SUPPORTS_PROCESSES
#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
#  define NS_SUPPORTS_PROCESSES NS_OFF
#else
#  define NS_SUPPORTS_PROCESSES NS_ON
#endif

// SIMD support
#undef NS_SIMD_IMPLEMENTATION

#if NS_ENABLED(NS_PLATFORM_ARCH_X86)
#  define NS_SIMD_IMPLEMENTATION NS_SIMD_IMPLEMENTATION_SSE
#elif NS_ENABLED(NS_PLATFORM_ARCH_ARM)
#  define NS_SIMD_IMPLEMENTATION NS_SIMD_IMPLEMENTATION_FPU
#else
#  error "Unknown architecture."
#endif

// Writing crashdumps is only supported on windows desktop
#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
#  undef NS_SUPPORTS_CRASH_DUMPS
#  define NS_SUPPORTS_CRASH_DUMPS NS_ON
#endif

// support for writing to files with very long paths is not implemented for UWP
#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
#  undef NS_SUPPORTS_LONG_PATHS
#  define NS_SUPPORTS_LONG_PATHS NS_ON
#endif
