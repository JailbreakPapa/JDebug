#pragma once

/// If set to 1, the POSIX file implementation will be used. Otherwise a platform specific implementation must be available.
#undef NS_USE_POSIX_FILE_API
#define NS_USE_POSIX_FILE_API NS_ON

/// Iterating through the file system is not supported
#undef NS_SUPPORTS_FILE_ITERATORS
#define NS_SUPPORTS_FILE_ITERATORS NS_OFF

/// Getting the stats of a file (modification times etc.) is supported.
#undef NS_SUPPORTS_FILE_STATS
#define NS_SUPPORTS_FILE_STATS NS_ON

/// Directory watcher is not supported
#undef NS_SUPPORTS_DIRECTORY_WATCHER
#define NS_SUPPORTS_DIRECTORY_WATCHER NS_OFF

/// Memory mapping a file is supported.
#undef NS_SUPPORTS_MEMORY_MAPPED_FILE
#define NS_SUPPORTS_MEMORY_MAPPED_FILE NS_ON

/// Shared memory IPC is supported.
#undef NS_SUPPORTS_SHARED_MEMORY
#define NS_SUPPORTS_SHARED_MEMORY NS_ON

/// Whether dynamic plugins (through DLLs loaded/unloaded at runtime) are supported
#undef NS_SUPPORTS_DYNAMIC_PLUGINS
#define NS_SUPPORTS_DYNAMIC_PLUGINS NS_OFF

/// Whether applications can access any file (not sandboxed)
#undef NS_SUPPORTS_UNRESTRICTED_FILE_ACCESS
#define NS_SUPPORTS_UNRESTRICTED_FILE_ACCESS NS_ON

/// Whether file accesses can be done through paths that do not match exact casing
#undef NS_SUPPORTS_CASE_INSENSITIVE_PATHS
#define NS_SUPPORTS_CASE_INSENSITIVE_PATHS NS_OFF

/// Whether writing to files with very long paths is supported / implemented
#undef NS_SUPPORTS_LONG_PATHS
#define NS_SUPPORTS_LONG_PATHS NS_ON

/// Whether starting other processes is supported.
#undef NS_SUPPORTS_PROCESSES
#define NS_SUPPORTS_PROCESSES NS_ON

// SIMD support
#undef NS_SIMD_IMPLEMENTATION
#define NS_SIMD_IMPLEMENTATION NS_SIMD_IMPLEMENTATION_FPU
