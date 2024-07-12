
#pragma once

#ifdef __clang__

#  undef NS_COMPILER_CLANG
#  define NS_COMPILER_CLANG NS_ON

#  define NS_ALWAYS_INLINE __attribute__((always_inline)) inline
#  if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
#    define NS_FORCE_INLINE inline
#  else
#    define NS_FORCE_INLINE __attribute__((always_inline)) inline
#  endif

#  define NS_ALIGNMENT_OF(type) NS_COMPILE_TIME_MAX(__alignof(type), NS_ALIGNMENT_MINIMUM)

#  if __has_builtin(__builtin_debugtrap)
#    define NS_DEBUG_BREAK     \
      {                        \
        __builtin_debugtrap(); \
      }
#  elif __has_builtin(__debugbreak)
#    define NS_DEBUG_BREAK \
      {                    \
        __debugbreak();    \
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

#  define NS_WARNING_PUSH() _Pragma("clang diagnostic push")
#  define NS_WARNING_POP() _Pragma("clang diagnostic pop")
#  define NS_WARNING_DISABLE_CLANG(_x) _Pragma(NS_STRINGIZE(clang diagnostic ignored _x))

#else

#  define NS_WARNING_DISABLE_CLANG(_x)

#endif
