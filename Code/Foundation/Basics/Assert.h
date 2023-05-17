#pragma once

/// \file

#include <Foundation/Basics.h>

/// ***** Assert Usage Guidelines *****
///
/// For your typical code, use WD_ASSERT_DEV to check that vital preconditions are met.
/// Be aware that WD_ASSERT_DEV is removed in non-development builds (ie. when WD_COMPILE_FOR_DEVELOPMENT is disabled),
/// INCLUDING your code in the assert condition.
/// If the code that you are checking must be executed, even in non-development builds, use WD_VERIFY instead.
/// WD_ASSERT_DEV and WD_VERIFY will trigger a breakpoint in debug builds, but will not interrupt the application
/// in release builds.
///
/// For conditions that are rarely violated or checking is very costly, use WD_ASSERT_DEBUG. This assert is only active
/// in debug builds. This allows to have extra checking while debugging a program, but not waste performance when a
/// development release build is used.
///
/// If you need to check something that is so vital that the application can only fail (i.e. crash), if that condition
/// is not met, even in release builds, then use WD_ASSERT_RELEASE. This should not be used in frequently executed code,
/// as it is not stripped from non-development builds by default.
///
/// If you need to squewde the last bit of performance out of your code, WD_ASSERT_RELEASE can be disabled, by defining
/// WD_DISABLE_RELEASE_ASSERTS.
/// Please be aware that WD_ASSERT_RELEASE works like the other asserts, i.e. once it is deactivated, the code in the condition
/// is not executed anymore.
///



/// \brief Assert handler callback. Should return true to trigger a break point or false if the assert should be ignored
using wdAssertHandler = bool (*)(
  const char* szSourceFile, wdUInt32 uiLine, const char* szFunction, const char* szExpression, const char* szAssertMsg);

WD_FOUNDATION_DLL bool wdDefaultAssertHandler(
  const char* szSourceFile, wdUInt32 uiLine, const char* szFunction, const char* szExpression, const char* szAssertMsg);

/// \brief Gets the current assert handler. The default assert handler shows a dialog on windows or prints to the console on other platforms.
WD_FOUNDATION_DLL wdAssertHandler wdGetAssertHandler();

/// \brief Sets the assert handler. It is the responsibility of the user to chain assert handlers if needed.
WD_FOUNDATION_DLL void wdSetAssertHandler(wdAssertHandler handler);

/// \brief Called by the assert macros whenever a check failed. Returns true if the user wants to trigger a break point
WD_FOUNDATION_DLL bool wdFailedCheck(
  const char* szSourceFile, wdUInt32 uiLine, const char* szFunction, const char* szExpression, const class wdFormatString& msg);
WD_FOUNDATION_DLL bool wdFailedCheck(const char* szSourceFile, wdUInt32 uiLine, const char* szFunction, const char* szExpression, const char* szMsg);

/// \brief Dummy version of wdFmt that only takes a single argument
inline const char* wdFmt(const char* szFormat)
{
  return szFormat;
}

#if WD_ENABLED(WD_COMPILER_MSVC)
// Hides the call to __debugbreak from MSVCs optimizer to work around a bug in VS 2019
// that can lead to code (memcpy) after an assert to be omitted
WD_FOUNDATION_DLL void MSVC_OutOfLine_DebugBreak(...);
#endif

/// \brief Macro to report a failure when that code is reached. This will ALWAYS be executed, even in release builds, therefore might crash the
/// application (or trigger a debug break).
#define WD_REPORT_FAILURE(szErrorMsg, ...)                                                                       \
  do                                                                                                             \
  {                                                                                                              \
    if (wdFailedCheck(WD_SOURCE_FILE, WD_SOURCE_LINE, WD_SOURCE_FUNCTION, "", wdFmt(szErrorMsg, ##__VA_ARGS__))) \
      WD_DEBUG_BREAK;                                                                                            \
  } while (false)

/// \brief Macro to raise an error, if a condition is not met. Allows to write a message using printf style. This assert will be triggered, even in
/// non-development builds and cannot be deactivated.
#define WD_ASSERT_ALWAYS(bCondition, szErrorMsg, ...)                                                                       \
  do                                                                                                                        \
  {                                                                                                                         \
    WD_MSVC_ANALYSIS_WARNING_PUSH                                                                                           \
    WD_MSVC_ANALYSIS_WARNING_DISABLE(6326) /* disable static analysis for the comparison */                                 \
    if (!!(bCondition) == false)                                                                                            \
    {                                                                                                                       \
      if (wdFailedCheck(WD_SOURCE_FILE, WD_SOURCE_LINE, WD_SOURCE_FUNCTION, #bCondition, wdFmt(szErrorMsg, ##__VA_ARGS__))) \
        WD_DEBUG_BREAK;                                                                                                     \
    }                                                                                                                       \
    WD_MSVC_ANALYSIS_WARNING_POP                                                                                            \
  } while (false)

/// \brief This type of assert can be used to mark code as 'not (yet) implemented' and makes it easier to find it later on by just searching for these
/// asserts.
#define WD_ASSERT_NOT_IMPLEMENTED WD_REPORT_FAILURE("Not implemented");

// Occurrences of WD_ASSERT_DEBUG are compiled out in non-debug builds
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
/// \brief Macro to raise an error, if a condition is not met.
///
/// Allows to write a message using printf style.
/// Compiled out in non-debug builds.
/// The condition is not evaluated, when this is compiled out, so do not execute important code in it.
#  define WD_ASSERT_DEBUG WD_ASSERT_ALWAYS
#else
/// \brief Macro to raise an error, if a condition is not met.
///
/// Allows to write a message using printf style.
/// Compiled out in non-debug builds.
/// The condition is not evaluated, when this is compiled out, so do not execute important code in it.
#  define WD_ASSERT_DEBUG(bCondition, szErrorMsg, ...)
#endif


// Occurrences of WD_ASSERT_DEV are compiled out in non-development builds
#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT) || WD_ENABLED(WD_COMPILE_FOR_DEBUG)

/// \brief Macro to raise an error, if a condition is not met.
///
/// Allows to write a message using printf style.
/// Compiled out in non-development builds.
/// The condition is not evaluated, when this is compiled out, so do not execute important code in it.
#  define WD_ASSERT_DEV WD_ASSERT_ALWAYS

/// \brief Macro to raise an error, if a condition is not met.
///
/// Allows to write a message using printf style.
/// Compiled out in non-development builds, however the condition is always evaluated,
/// so you may execute important code in it.
#  define WD_VERIFY WD_ASSERT_ALWAYS

#else

/// \brief Macro to raise an error, if a condition is not met.
///
/// Allows to write a message using printf style.
/// Compiled out in non-development builds.
/// The condition is not evaluated, when this is compiled out, so do not execute important code in it.
#  define WD_ASSERT_DEV(bCondition, szErrorMsg, ...)

/// \brief Macro to raise an error, if a condition is not met.
///
/// Allows to write a message using printf style.
/// Compiled out in non-development builds, however the condition is always evaluated,
/// so you may execute important code in it.
#  define WD_VERIFY(bCondition, szErrorMsg, ...)                             \
    if (!!(bCondition) == false)                                             \
    { /* The condition is evaluated, even though nothing is done with it. */ \
    }

#endif

#if WD_DISABLE_RELEASE_ASSERTS

/// \brief An assert to check conditions even in release builds.
///
/// These asserts can be disabled (and then their condition will not be evaluated),
/// but this needs to be specifically done by the user by defining WD_DISABLE_RELEASE_ASSERTS.
/// That should only be done, if you are intending to ship a product, and want get rid of all unnecessary overhead.
#  define WD_ASSERT_RELEASE(bCondition, szErrorMsg, ...)

#else

/// \brief An assert to check conditions even in release builds.
///
/// These asserts can be disabled (and then their condition will not be evaluated),
/// but this needs to be specifically done by the user by defining WD_DISABLE_RELEASE_ASSERTS.
/// That should only be done, if you are intending to ship a product, and want get rid of all unnecessary overhead.
#  define WD_ASSERT_RELEASE WD_ASSERT_ALWAYS

#endif

/// \brief Macro to make unhandled cases in a switch block an error.
#define WD_DEFAULT_CASE_NOT_IMPLEMENTED \
  default:                              \
    WD_ASSERT_NOT_IMPLEMENTED           \
    break;
