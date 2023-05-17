
#pragma once

#ifdef __clang__

#  undef WD_COMPILER_CLANG
#  define WD_COMPILER_CLANG WD_ON

#  define WD_ALWAYS_INLINE __attribute__((always_inline)) inline
#  if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
#    define WD_FORCE_INLINE inline
#  else
#    define WD_FORCE_INLINE __attribute__((always_inline)) inline
#  endif

#  define WD_ALIGNMENT_OF(type) WD_COMPILE_TIME_MAX(__alignof(type), WD_ALIGNMENT_MINIMUM)

#  define WD_DEBUG_BREAK \
    {                    \
      __builtin_trap();  \
    }

#  define WD_SOURCE_FUNCTION __PRETTY_FUNCTION__
#  define WD_SOURCE_LINE __LINE__
#  define WD_SOURCE_FILE __FILE__

#  ifdef BUILDSYSTEM_BUILDTYPE_Debug
#    undef WD_COMPILE_FOR_DEBUG
#    define WD_COMPILE_FOR_DEBUG WD_ON
#  endif

#endif
