#pragma once

// On MSVC 2008 in 64 Bit <cmath> generates a lot of warnings (actually it is math.h, which is included by cmath)
#define WD_MSVC_WARNING_NUMBER 4985
#include <Foundation/Basics/Compiler/MSVC/DisableWarning_MSVC.h>

// include std header
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <new>

#include <Foundation/Basics/Compiler/MSVC/RestoreWarning_MSVC.h>

// redefine NULL to nullptr
#undef NULL
#define NULL nullptr

// include c++11 specific header
#include <type_traits>
#include <utility>

#ifndef __has_cpp_attribute
#  define __has_cpp_attribute(name) 0
#endif

// [[nodiscard]] helper
#if __has_cpp_attribute(nodiscard)
#  define WD_NODISCARD [[nodiscard]]
#else
#  define WD_NODISCARD
#endif

#ifndef __INTELLISENSE__

// Macros to do compile-time checks, such as to ensure sizes of types
// WD_CHECK_AT_COMPILETIME(exp) : only checks exp
// WD_CHECK_AT_COMPILETIME_MSG(exp, msg) : checks exp and displays msg
#  define WD_CHECK_AT_COMPILETIME(exp) static_assert(exp, WD_STRINGIZE(exp) " is false.");

#  define WD_CHECK_AT_COMPILETIME_MSG(exp, msg) static_assert(exp, WD_STRINGIZE(exp) " is false. Message: " msg);

#else

// IntelliSense often isn't smart enough to evaluate these conditions correctly

#  define WD_CHECK_AT_COMPILETIME(exp)

#  define WD_CHECK_AT_COMPILETIME_MSG(exp, msg)

#endif

/// \brief Disallow the copy constructor and the assignment operator for this type.
#define WD_DISALLOW_COPY_AND_ASSIGN(type) \
  type(const type&) = delete;             \
  void operator=(const type&) = delete

#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
/// \brief Macro helper to check alignment
#  define WD_CHECK_ALIGNMENT(ptr, alignment) WD_ASSERT_DEV(((size_t)ptr & (alignment - 1)) == 0, "Wrong alignment.")
#else
/// \brief Macro helper to check alignment
#  define WD_CHECK_ALIGNMENT(ptr, alignment)
#endif

#define WD_CHECK_ALIGNMENT_16(ptr) WD_CHECK_ALIGNMENT(ptr, 16)
#define WD_CHECK_ALIGNMENT_32(ptr) WD_CHECK_ALIGNMENT(ptr, 32)
#define WD_CHECK_ALIGNMENT_64(ptr) WD_CHECK_ALIGNMENT(ptr, 64)
#define WD_CHECK_ALIGNMENT_128(ptr) WD_CHECK_ALIGNMENT(ptr, 128)

#define WD_WINCHECK_1 1          // WD_INCLUDED_WINDOWS_H defined to 1, _WINDOWS_ defined (stringyfied to nothing)
#define WD_WINCHECK_1_WINDOWS_ 1 // WD_INCLUDED_WINDOWS_H defined to 1, _WINDOWS_ undefined (stringyfied to "_WINDOWS_")
#define WD_WINCHECK_WD_INCLUDED_WINDOWS_H \
  0 // WD_INCLUDED_WINDOWS_H undefined (stringyfied to "WD_INCLUDED_WINDOWS_H", _WINDOWS_ defined (stringyfied to nothing)
#define WD_WINCHECK_WD_INCLUDED_WINDOWS_H_WINDOWS_ \
  1 // WD_INCLUDED_WINDOWS_H undefined (stringyfied to "WD_INCLUDED_WINDOWS_H", _WINDOWS_ undefined (stringyfied to "_WINDOWS_")

/// \brief Checks whether Windows.h has been included directly instead of through 'IncludeWindows.h'
///
/// Does this by stringifying the available defines, concatenating them into one long word, which is a known #define that evaluates to 0 or 1
#define WD_CHECK_WINDOWS_INCLUDE(WD_WINH_INCLUDED, WINH_INCLUDED)                                       \
  WD_CHECK_AT_COMPILETIME_MSG(WD_CONCAT(WD_WINCHECK_, WD_CONCAT(WD_WINH_INCLUDED, WINH_INCLUDED)) == 1, \
    "Windows.h has been included but not through wd. #include <Foundation/Basics/Platform/Win/IncludeWindows.h> instead of Windows.h");


/// \brief Define some macros to work with the MSVC analysis warning
/// Note that the StaticAnalysis.h in Basics/Compiler/MSVC will define the MSVC specific versions.
#define WD_MSVC_ANALYSIS_WARNING_PUSH
#define WD_MSVC_ANALYSIS_WARNING_POP
#define WD_MSVC_ANALYSIS_WARNING_DISABLE(warningNumber)
#define WD_MSVC_ANALYSIS_ASSUME(expression)

#if defined(_MSC_VER)
#  include <Foundation/Basics/Compiler/MSVC/StaticAnalysis.h>
#endif

#if WD_ENABLED(WD_COMPILE_ENGINE_AS_DLL)

/// \brief The tool 'StaticLinkUtil' inserts this macro into each file in a library.
/// Each library also needs to contain exactly one instance of WD_STATICLINK_LIBRARY.
/// The macros create functions that reference each other, which means the linker is forced to look at all files in the library.
/// This in turn will drag all global variables into the visibility of the linker, and since it mustn't optimize them away,
/// they then end up in the final application, where they will do what they are meant for.
#  define WD_STATICLINK_FILE(LibraryName, UniqueName) WD_CHECK_WINDOWS_INCLUDE(WD_INCLUDED_WINDOWS_H, _WINDOWS_)


/// \brief Used by the tool 'StaticLinkUtil' to generate the block after WD_STATICLINK_LIBRARY, to create references to all
/// files inside a library. \see WD_STATICLINK_FILE
#  define WD_STATICLINK_REFERENCE(UniqueName)

/// \brief This must occur exactly once in each static library, such that all WD_STATICLINK_FILE macros can reference it.
#  define WD_STATICLINK_LIBRARY(LibraryName) void wdReferenceFunction_##LibraryName(bool bReturn = true)

#else

struct wdStaticLinkHelper
{
  typedef void (*Func)(bool);
  wdStaticLinkHelper(Func f) { f(true); }
};

/// \brief The tool 'StaticLinkUtil' inserts this macro into each file in a library.
/// Each library also needs to contain exactly one instance of WD_STATICLINK_LIBRARY.
/// The macros create functions that reference each other, which means the linker is forced to look at all files in the library.
/// This in turn will drag all global variables into the visibility of the linker, and since it mustn't optimize them away,
/// they then end up in the final application, where they will do what they are meant for.
#  define WD_STATICLINK_FILE(LibraryName, UniqueName)      \
    void wdReferenceFunction_##UniqueName(bool bReturn) {} \
    void wdReferenceFunction_##LibraryName(bool bReturn);  \
    static wdStaticLinkHelper StaticLinkHelper_##UniqueName(wdReferenceFunction_##LibraryName);

/// \brief Used by the tool 'StaticLinkUtil' to generate the block after WD_STATICLINK_LIBRARY, to create references to all
/// files inside a library. \see WD_STATICLINK_FILE
#  define WD_STATICLINK_REFERENCE(UniqueName)                   \
    void wdReferenceFunction_##UniqueName(bool bReturn = true); \
    wdReferenceFunction_##UniqueName()

/// \brief This must occur exactly once in each static library, such that all WD_STATICLINK_FILE macros can reference it.
#  define WD_STATICLINK_LIBRARY(LibraryName) void wdReferenceFunction_##LibraryName(bool bReturn = true)

#endif

namespace wdInternal
{
  template <typename T, size_t N>
  char (*ArraySizeHelper(T (&)[N]))[N];
}

/// \brief Macro to determine the size of a static array
#define WD_ARRAY_SIZE(a) (sizeof(*wdInternal::ArraySizeHelper(a)) + 0)

/// \brief Template helper which allows to suppress "Unused variable" warnings (e.g. result used in platform specific block, ..)
template <class T>
void WD_IGNORE_UNUSED(const T&)
{
}


// Math Debug checks
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)

#  undef WD_MATH_CHECK_FOR_NAN
#  define WD_MATH_CHECK_FOR_NAN WD_ON

#endif

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  define WD_DECL_EXPORT __declspec(dllexport)
#  define WD_DECL_IMPORT __declspec(dllimport)
#  define WD_DECL_EXPORT_FRIEND __declspec(dllexport)
#  define WD_DECL_IMPORT_FRIEND __declspec(dllimport)
#else
#  define WD_DECL_EXPORT [[gnu::visibility("default")]]
#  define WD_DECL_IMPORT [[gnu::visibility("default")]]
#  define WD_DECL_EXPORT_FRIEND
#  define WD_DECL_IMPORT_FRIEND
#endif
