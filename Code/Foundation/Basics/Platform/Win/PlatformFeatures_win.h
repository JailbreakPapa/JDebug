#pragma once

/// \file

/// If set to 1, the POSIX file implementation will be used. Otherwise a platform specific implementation must be available.
#undef WD_USE_POSIX_FILE_API

#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
#  define WD_USE_POSIX_FILE_API WD_ON
#else
#  define WD_USE_POSIX_FILE_API WD_OFF
#endif

/// Iterating through the file system is supported
#undef WD_SUPPORTS_FILE_ITERATORS

#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
#  define WD_SUPPORTS_FILE_ITERATORS WD_OFF
#else
#  define WD_SUPPORTS_FILE_ITERATORS WD_ON
#endif

/// Getting the stats of a file (modification times etc.) is supported.
#undef WD_SUPPORTS_FILE_STATS
#define WD_SUPPORTS_FILE_STATS WD_ON

/// Directory watcher is supported on non uwp platforms.
#undef WD_SUPPORTS_DIRECTORY_WATCHER
#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
#  define WD_SUPPORTS_DIRECTORY_WATCHER WD_OFF
#else
#  define WD_SUPPORTS_DIRECTORY_WATCHER WD_ON
#endif

/// Memory mapping a file is supported.
#undef WD_SUPPORTS_MEMORY_MAPPED_FILE
#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
#  define WD_SUPPORTS_MEMORY_MAPPED_FILE WD_OFF
#else
#  define WD_SUPPORTS_MEMORY_MAPPED_FILE WD_ON
#endif

/// Shared memory IPC is supported.
#undef WD_SUPPORTS_SHARED_MEMORY
#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
#  define WD_SUPPORTS_SHARED_MEMORY WD_OFF
#else
#  define WD_SUPPORTS_SHARED_MEMORY WD_ON
#endif

/// Whether dynamic plugins (through DLLs loaded/unloaded at runtime) are supported
#undef WD_SUPPORTS_DYNAMIC_PLUGINS
#define WD_SUPPORTS_DYNAMIC_PLUGINS WD_ON

/// Whether applications can access any file (not sandboxed)
#undef WD_SUPPORTS_UNRESTRICTED_FILE_ACCESS
#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
#  define WD_SUPPORTS_UNRESTRICTED_FILE_ACCESS WD_OFF
#else
#  define WD_SUPPORTS_UNRESTRICTED_FILE_ACCESS WD_ON
#endif

/// Whether file accesses can be done through paths that do not match exact casing
#undef WD_SUPPORTS_CASE_INSENSITIVE_PATHS
#define WD_SUPPORTS_CASE_INSENSITIVE_PATHS WD_ON

/// Whether starting other processes is supported.
#undef WD_SUPPORTS_PROCESSES
#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
#  define WD_SUPPORTS_PROCESSES WD_OFF
#else
#  define WD_SUPPORTS_PROCESSES WD_ON
#endif

// SIMD support
#undef WD_SIMD_IMPLEMENTATION

#if WD_ENABLED(WD_PLATFORM_ARCH_X86)
#  define WD_SIMD_IMPLEMENTATION WD_SIMD_IMPLEMENTATION_SSE
#elif WD_ENABLED(WD_PLATFORM_ARCH_ARM)
#  define WD_SIMD_IMPLEMENTATION WD_SIMD_IMPLEMENTATION_FPU
#else
#  error "Unknown architecture."
#endif

// Writing crashdumps is only supported on windows desktop
#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
#  undef WD_SUPPORTS_CRASH_DUMPS
#  define WD_SUPPORTS_CRASH_DUMPS WD_ON
#endif

// support for writing to files with very long paths is not implemented for UWP
#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
#  undef WD_SUPPORTS_LONG_PATHS
#  define WD_SUPPORTS_LONG_PATHS WD_ON
#endif
