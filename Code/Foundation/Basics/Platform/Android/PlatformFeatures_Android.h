#pragma once

/// If set to 1, the POSIX file implementation will be used. Otherwise a platform specific implementation must be available.
#undef WD_USE_POSIX_FILE_API
#define WD_USE_POSIX_FILE_API WD_ON

/// Iterating through the file system is not supported
#undef WD_SUPPORTS_FILE_ITERATORS
#define WD_SUPPORTS_FILE_ITERATORS WD_OFF

/// Directory watcher is not supported
#undef WD_SUPPORTS_DIRECTORY_WATCHER
#define WD_SUPPORTS_DIRECTORY_WATCHER WD_OFF

/// Getting the stats of a file (modification times etc.) is supported.
#undef WD_SUPPORTS_FILE_STATS
#define WD_SUPPORTS_FILE_STATS WD_ON

/// Memory mapping a file is supported.
#undef WD_SUPPORTS_MEMORY_MAPPED_FILE
#define WD_SUPPORTS_MEMORY_MAPPED_FILE WD_ON

/// Shared memory IPC is not supported.
/// shm_open / shm_unlink deprecated.
/// There is an alternative in ASharedMemory_create but that is only
/// available in API 26 upwards.
/// Could be implemented via JNI which defeats the purpose of a fast IPC channel
/// or we could just use an actual file as the shared memory block.
#undef WD_SUPPORTS_SHARED_MEMORY
#define WD_SUPPORTS_SHARED_MEMORY WD_OFF

/// Whether dynamic plugins (through DLLs loaded/unloaded at runtime) are supported
#undef WD_SUPPORTS_DYNAMIC_PLUGINS
#define WD_SUPPORTS_DYNAMIC_PLUGINS WD_OFF

/// Whether applications can access any file (not sandboxed)
#undef WD_SUPPORTS_UNRESTRICTED_FILE_ACCESS
#define WD_SUPPORTS_UNRESTRICTED_FILE_ACCESS WD_OFF

/// Whether file accesses can be done through paths that do not match exact casing
#undef WD_SUPPORTS_CASE_INSENSITIVE_PATHS
#define WD_SUPPORTS_CASE_INSENSITIVE_PATHS WD_OFF

/// Whether writing to files with very long paths is supported / implemented
#undef WD_SUPPORTS_LONG_PATHS
#define WD_SUPPORTS_LONG_PATHS WD_ON

/// Whether starting other processes is supported.
#undef WD_SUPPORTS_PROCESSES
#define WD_SUPPORTS_PROCESSES WD_ON

// SIMD support
#undef WD_SIMD_IMPLEMENTATION
#define WD_SIMD_IMPLEMENTATION WD_SIMD_IMPLEMENTATION_FPU
