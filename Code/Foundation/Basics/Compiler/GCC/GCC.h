
#pragma once

#if !defined(__clang__) && (defined(__GNUC__) || defined(__GNUG__))

#  undef WD_COMPILER_GCC
#  define WD_COMPILER_GCC WD_ON

/// \todo re-investigate: attribute(always inline) does not work for some reason
#  define WD_ALWAYS_INLINE inline
#  define WD_FORCE_INLINE inline

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
