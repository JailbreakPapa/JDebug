#pragma once

#if defined(_MSC_VER) && !defined(__clang__)

#  undef WD_COMPILER_MSVC
#  define WD_COMPILER_MSVC WD_ON

#  if __clang__ || __castxml__
#    undef WD_COMPILER_MSVC_CLANG
#    define WD_COMPILER_MSVC_CLANG WD_ON
#  else
#    undef WD_COMPILER_MSVC_PURE
#    define WD_COMPILER_MSVC_PURE WD_ON
#  endif

#  ifdef _DEBUG
#    undef WD_COMPILE_FOR_DEBUG
#    define WD_COMPILE_FOR_DEBUG WD_ON
#  endif


// Functions marked as WD_ALWAYS_INLINE will be inlined even in Debug builds, which means you will step over them in a debugger
#  define WD_ALWAYS_INLINE __forceinline

#  if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
#    define WD_FORCE_INLINE inline
#  else
#    define WD_FORCE_INLINE __forceinline
#  endif

// workaround for MSVC compiler issue with alignment determination of dependent types
#  define WD_ALIGNMENT_OF(type) WD_COMPILE_TIME_MAX(WD_ALIGNMENT_MINIMUM, WD_COMPILE_TIME_MIN(sizeof(type), __alignof(type)))

#  if WD_ENABLED(WD_COMPILE_FOR_DEBUG) || (_MSC_VER >= 1929 /* broken in early VS2019 but works again in VS2022 and later 2019 versions*/)

#    define WD_DEBUG_BREAK \
      {                    \
        __debugbreak();    \
      }

#  else

#    define WD_DEBUG_BREAK                         \
      {                                            \
        /* Declared with DLL export in Assert.h */ \
        MSVC_OutOfLine_DebugBreak();               \
      }

#  endif

#  if WD_ENABLED(WD_COMPILER_MSVC_CLANG)
#    define WD_SOURCE_FUNCTION __PRETTY_FUNCTION__
#  else
#    define WD_SOURCE_FUNCTION __FUNCTION__
#  endif

#  define WD_SOURCE_LINE __LINE__
#  define WD_SOURCE_FILE __FILE__

// WD_VA_NUM_ARGS() is a very nifty macro to retrieve the number of arguments handed to a variable-argument macro
// unfortunately, VS 2010 still has this compiler bug which treats a __VA_ARGS__ argument as being one single parameter:
// https://connect.microsoft.com/VisualStudio/feedback/details/521844/variadic-macro-treating-va-args-as-a-single-parameter-for-other-macros#details
#  if _MSC_VER >= 1400 && WD_DISABLED(WD_COMPILER_MSVC_CLANG)
#    define WD_VA_NUM_ARGS_HELPER(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, N, ...) N
#    define WD_VA_NUM_ARGS_REVERSE_SEQUENCE 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1
#    define WD_LEFT_PARENTHESIS (
#    define WD_RIGHT_PARENTHESIS )
#    define WD_VA_NUM_ARGS(...) WD_VA_NUM_ARGS_HELPER WD_LEFT_PARENTHESIS __VA_ARGS__, WD_VA_NUM_ARGS_REVERSE_SEQUENCE WD_RIGHT_PARENTHESIS
#  endif

#endif
