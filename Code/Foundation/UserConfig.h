#pragma once

/// \file

/// Global settings for how to to compile NS.
/// Modify these settings as you needed in your project.


#ifdef BUILDSYSTEM_COMPILE_ENGINE_AS_DLL
#  undef NS_COMPILE_ENGINE_AS_DLL
#  define NS_COMPILE_ENGINE_AS_DLL NS_ON
#else
#  undef NS_COMPILE_ENGINE_AS_DLL
#  define NS_COMPILE_ENGINE_AS_DLL NS_OFF
#endif

#if defined(BUILDSYSTEM_BUILDTYPE_Shipping)

// Development checks like assert.
#  undef NS_COMPILE_FOR_DEVELOPMENT
#  define NS_COMPILE_FOR_DEVELOPMENT NS_OFF

// Performance profiling features
#  undef NS_USE_PROFILING
#  define NS_USE_PROFILING NS_OFF

// Tracking of memory allocations.
#  undef NS_ALLOC_TRACKING_DEFAULT
#  define NS_ALLOC_TRACKING_DEFAULT nsAllocatorTrackingMode::Nothing

#else

// Development checks like assert.
#  undef NS_COMPILE_FOR_DEVELOPMENT
#  define NS_COMPILE_FOR_DEVELOPMENT NS_ON

// Performance profiling features
#  undef NS_USE_PROFILING
#  define NS_USE_PROFILING NS_ON

// Tracking of memory allocations.
#  undef NS_ALLOC_TRACKING_DEFAULT
#  define NS_ALLOC_TRACKING_DEFAULT nsAllocatorTrackingMode::AllocationStatsAndStacktraces

#endif

#if defined(BUILDSYSTEM_BUILDTYPE_Debug)
#  undef NS_MATH_CHECK_FOR_NAN
#  define NS_MATH_CHECK_FOR_NAN NS_ON
#  undef NS_USE_STRING_VALIDATION
#  define NS_USE_STRING_VALIDATION NS_ON
#endif


/// Whether game objects compute and store their velocity since the last frame (increases object size)
#define NS_GAMEOBJECT_VELOCITY NS_ON

// Migration code path. Added in March 2023, should be removed after a 'save' time.
#undef NS_MIGRATE_RUNTIMECONFIGS
#define NS_MIGRATE_RUNTIMECONFIGS NS_ON
