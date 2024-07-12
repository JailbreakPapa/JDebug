#pragma once

/// If set to 1, the POSIX file implementation will be used. Otherwise a platform specific implementation must be available.
#undef NS_USE_POSIX_FILE_API
#define NS_USE_POSIX_FILE_API NS_ON

/// If set to one linux posix extensions such as pipe2, dup3, etc are used.
#undef NS_USE_LINUX_POSIX_EXTENSIONS
#define NS_USE_LINUX_POSIX_EXTENSIONS NS_ON

/// Iterating through the file system is not supported
#undef NS_SUPPORTS_FILE_ITERATORS
#define NS_SUPPORTS_FILE_ITERATORS NS_OFF

/// Directory watcher is not supported
#undef NS_SUPPORTS_DIRECTORY_WATCHER
#define NS_SUPPORTS_DIRECTORY_WATCHER NS_OFF

/// Getting the stats of a file (modification times etc.) is supported.
#undef NS_SUPPORTS_FILE_STATS
#define NS_SUPPORTS_FILE_STATS NS_ON

/// Memory mapping a file is supported.
#undef NS_SUPPORTS_MEMORY_MAPPED_FILE
#define NS_SUPPORTS_MEMORY_MAPPED_FILE NS_ON

/// Shared memory IPC is not supported.
/// shm_open / shm_unlink deprecated.
/// There is an alternative in ASharedMemory_create but that is only
/// available in API 26 upwards.
/// Could be implemented via JNI which defeats the purpose of a fast IPC channel
/// or we could just use an actual file as the shared memory block.
#undef NS_SUPPORTS_SHARED_MEMORY
#define NS_SUPPORTS_SHARED_MEMORY NS_OFF

/// Whether dynamic plugins (through DLLs loaded/unloaded at runtime) are supported
#undef NS_SUPPORTS_DYNAMIC_PLUGINS
#define NS_SUPPORTS_DYNAMIC_PLUGINS NS_OFF

/// Whether applications can access any file (not sandboxed)
#undef NS_SUPPORTS_UNRESTRICTED_FILE_ACCESS
#define NS_SUPPORTS_UNRESTRICTED_FILE_ACCESS NS_OFF

/// Whether file accesses can be done through paths that do not match exact casing
#undef NS_SUPPORTS_CASE_INSENSITIVE_PATHS
#define NS_SUPPORTS_CASE_INSENSITIVE_PATHS NS_OFF

/// Whether writing to files with very long paths is supported / implemented
#undef NS_SUPPORTS_LONG_PATHS
#define NS_SUPPORTS_LONG_PATHS NS_ON

/// Whether starting other processes is supported.
#undef NS_SUPPORTS_PROCESSES
#define NS_SUPPORTS_PROCESSES NS_OFF

// SIMD support
#undef NS_SIMD_IMPLEMENTATION

#if NS_ENABLED(NS_PLATFORM_ARCH_X86)
#  if __SSE4_1__ && __SSSE3__
#    define NS_SIMD_IMPLEMENTATION NS_SIMD_IMPLEMENTATION_SSE
#  else
#    define NS_SIMD_IMPLEMENTATION NS_SIMD_IMPLEMENTATION_FPU
#  endif
#elif NS_ENABLED(NS_PLATFORM_ARCH_ARM)
#  if NS_ENABLED(NS_PLATFORM_64BIT)
#    define NS_SIMD_IMPLEMENTATION NS_SIMD_IMPLEMENTATION_NEON
#  else
#    define NS_SIMD_IMPLEMENTATION NS_SIMD_IMPLEMENTATION_FPU
#  endif
#else
#  error "Unknown architecture."
#endif
