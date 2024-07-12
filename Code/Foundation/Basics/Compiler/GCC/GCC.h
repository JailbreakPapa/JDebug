
#pragma once

#if !defined(__clang__) && (defined(__GNUC__) || defined(__GNUG__))

#  undef NS_COMPILER_GCC
#  define NS_COMPILER_GCC NS_ON

/// \todo re-investigate: attribute(always inline) does not work for some reason
#  define NS_ALWAYS_INLINE inline
#  define NS_FORCE_INLINE inline

#  define NS_ALIGNMENT_OF(type) NS_COMPILE_TIME_MAX(__alignof(type), NS_ALIGNMENT_MINIMUM)

#  if __has_builtin(__builtin_debugtrap)
#    define NS_DEBUG_BREAK     \
      {                        \
        __builtin_debugtrap(); \
      }
#  elif defined(__i386__) || defined(__x86_64__)
#    define NS_DEBUG_BREAK            \
      {                               \
        __asm__ __volatile__("int3"); \
      }
#  else
#    include <signal.h>
#    if defined(SIGTRAP)
#      define NS_DEBUG_BREAK \
        {                    \
          raise(SIGTRAP);    \
        }
#    else
#      define NS_DEBUG_BREAK \
        {                    \
          raise(SIGABRT);    \
        }
#    endif
#  endif

#  define NS_SOURCE_FUNCTION __PRETTY_FUNCTION__
#  define NS_SOURCE_LINE __LINE__
#  define NS_SOURCE_FILE __FILE__

#  ifdef BUILDSYSTEM_BUILDTYPE_Debug
#    undef NS_COMPILE_FOR_DEBUG
#    define NS_COMPILE_FOR_DEBUG NS_ON
#  endif

#  define NS_WARNING_PUSH() _Pragma("GCC diagnostic push")
#  define NS_WARNING_POP() _Pragma("GCC diagnostic pop")
#  define NS_WARNING_DISABLE_GCC(_x) _Pragma(NS_STRINGIZE(GCC diagnostic ignored _x))

#else

#  define NS_WARNING_DISABLE_GCC(_x)

#endif
