#pragma once

/// \file

/// Global settings for how to to compile WD.
/// Modify these settings as you needed in your project.


#ifdef BUILDSYSTEM_COMPILE_ENGINE_AS_DLL
#  undef WD_COMPILE_ENGINE_AS_DLL
#  define WD_COMPILE_ENGINE_AS_DLL WD_ON
#else
#  undef WD_COMPILE_ENGINE_AS_DLL
#  define WD_COMPILE_ENGINE_AS_DLL WD_OFF
#endif

#if defined(BUILDSYSTEM_BUILDTYPE_Shipping)

// Development checks like assert.
#  undef WD_COMPILE_FOR_DEVELOPMENT
#  define WD_COMPILE_FOR_DEVELOPMENT WD_OFF

// Performance profiling features
#  undef WD_USE_PROFILING
#  define WD_USE_PROFILING WD_OFF

// Tracking of memory allocations.
#  undef WD_USE_ALLOCATION_TRACKING
#  define WD_USE_ALLOCATION_TRACKING WD_OFF

// Stack traces for memory allocations.
#  undef WD_USE_ALLOCATION_STACK_TRACING
#  define WD_USE_ALLOCATION_STACK_TRACING WD_OFF

#else

// Development checks like assert.
#  undef WD_COMPILE_FOR_DEVELOPMENT
#  define WD_COMPILE_FOR_DEVELOPMENT WD_ON

// Performance profiling features
#  undef WD_USE_PROFILING
#  define WD_USE_PROFILING WD_ON

// Tracking of memory allocations.
#  undef WD_USE_ALLOCATION_TRACKING
#  define WD_USE_ALLOCATION_TRACKING WD_ON

// Stack traces for memory allocations.
#  undef WD_USE_ALLOCATION_STACK_TRACING
#  define WD_USE_ALLOCATION_STACK_TRACING WD_ON

#endif

/// Whether game objects compute and store their velocity since the last frame (increases object size)
#define WD_GAMEOBJECT_VELOCITY WD_ON

// Migration code path. Added in March 2023, should be removed after a 'save' time.
#undef WD_MIGRATE_RUNTIMECONFIGS
#define WD_MIGRATE_RUNTIMECONFIGS WD_ON
