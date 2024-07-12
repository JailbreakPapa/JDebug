#pragma once

// On MSVC 2008 in 64 Bit <cmath> generates a lot of warnings (actually it is math.h, which is included by cmath)
NS_WARNING_PUSH()
NS_WARNING_DISABLE_MSVC(4985)

// include std header
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <new>

NS_WARNING_POP()

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
#  define NS_NODISCARD [[nodiscard]]
#else
#  define NS_NODISCARD
#endif

#ifndef __INTELLISENSE__

// Macros to do compile-time checks, such as to ensure sizes of types
// NS_CHECK_AT_COMPILETIME(exp) : only checks exp
// NS_CHECK_AT_COMPILETIME_MSG(exp, msg) : checks exp and displays msg
#  define NS_CHECK_AT_COMPILETIME(exp) static_assert(exp, NS_STRINGIZE(exp) " is false.");

#  define NS_CHECK_AT_COMPILETIME_MSG(exp, msg) static_assert(exp, NS_STRINGIZE(exp) " is false. Message: " msg);

#else

// IntelliSense often isn't smart enough to evaluate these conditions correctly

#  define NS_CHECK_AT_COMPILETIME(exp)

#  define NS_CHECK_AT_COMPILETIME_MSG(exp, msg)

#endif

/// \brief Disallow the copy constructor and the assignment operator for this type.
#define NS_DISALLOW_COPY_AND_ASSIGN(type) \
  type(const type&) = delete;             \
  void operator=(const type&) = delete

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
/// \brief Macro helper to check alignment
#  define NS_CHECK_ALIGNMENT(ptr, alignment) NS_ASSERT_DEV(((size_t)ptr & ((alignment) - 1)) == 0, "Wrong alignment.")
#else
/// \brief Macro helper to check alignment
#  define NS_CHECK_ALIGNMENT(ptr, alignment)
#endif

#define NS_CHECK_ALIGNMENT_16(ptr) NS_CHECK_ALIGNMENT(ptr, 16)
#define NS_CHECK_ALIGNMENT_32(ptr) NS_CHECK_ALIGNMENT(ptr, 32)
#define NS_CHECK_ALIGNMENT_64(ptr) NS_CHECK_ALIGNMENT(ptr, 64)
#define NS_CHECK_ALIGNMENT_128(ptr) NS_CHECK_ALIGNMENT(ptr, 128)

#define NS_WINCHECK_1 1          // NS_INCLUDED_WINDOWS_H defined to 1, _WINDOWS_ defined (stringyfied to nothing)
#define NS_WINCHECK_1_WINDOWS_ 1 // NS_INCLUDED_WINDOWS_H defined to 1, _WINDOWS_ undefined (stringyfied to "_WINDOWS_")
#define NS_WINCHECK_NS_INCLUDED_WINDOWS_H \
  0                              // NS_INCLUDED_WINDOWS_H undefined (stringyfied to "NS_INCLUDED_WINDOWS_H", _WINDOWS_ defined (stringyfied to nothing)
#define NS_WINCHECK_NS_INCLUDED_WINDOWS_H_WINDOWS_ \
  1                              // NS_INCLUDED_WINDOWS_H undefined (stringyfied to "NS_INCLUDED_WINDOWS_H", _WINDOWS_ undefined (stringyfied to "_WINDOWS_")

/// \brief Checks whether Windows.h has been included directly instead of through 'IncludeWindows.h'
///
/// Does this by stringifying the available defines, concatenating them into one long word, which is a known #define that evaluates to 0 or 1
#define NS_CHECK_WINDOWS_INCLUDE(NS_WINH_INCLUDED, WINH_INCLUDED)                                       \
  NS_CHECK_AT_COMPILETIME_MSG(NS_CONCAT(NS_WINCHECK_, NS_CONCAT(NS_WINH_INCLUDED, WINH_INCLUDED)) == 1, \
    "Windows.h has been included but not through ns. #include <Foundation/Basics/Platform/Win/IncludeWindows.h> instead of Windows.h");


/// \brief Define some macros to work with the MSVC analysis warning
/// Note that the StaticAnalysis.h in Basics/Compiler/MSVC will define the MSVC specific versions.
#define NS_MSVC_ANALYSIS_WARNING_PUSH
#define NS_MSVC_ANALYSIS_WARNING_POP
#define NS_MSVC_ANALYSIS_WARNING_DISABLE(warningNumber)
#define NS_MSVC_ANALYSIS_ASSUME(expression)

#if defined(_MSC_VER)
#  include <Foundation/Basics/Compiler/MSVC/StaticAnalysis.h>
#endif

#if NS_ENABLED(NS_COMPILE_ENGINE_AS_DLL)

/// \brief The tool 'StaticLinkUtil' inserts this macro into each file in a library.
/// Each library also needs to contain exactly one instance of NS_STATICLINK_LIBRARY.
/// The macros create functions that reference each other, which means the linker is forced to look at all files in the library.
/// This in turn will drag all global variables into the visibility of the linker, and since it mustn't optimize them away,
/// they then end up in the final application, where they will do what they are meant for.
#  define NS_STATICLINK_FILE(LibraryName, UniqueName) NS_CHECK_WINDOWS_INCLUDE(NS_INCLUDED_WINDOWS_H, _WINDOWS_)

/// \brief Used by the tool 'StaticLinkUtil' to generate the block after NS_STATICLINK_LIBRARY, to create references to all
/// files inside a library. \see NS_STATICLINK_FILE
#  define NS_STATICLINK_REFERENCE(UniqueName)

/// \brief This must occur exactly once in each static library, such that all NS_STATICLINK_FILE macros can reference it.
#  define NS_STATICLINK_LIBRARY(LibraryName) void nsReferenceFunction_##LibraryName(bool bReturn = true)

#else

struct nsStaticLinkHelper
{
  using Func = void (*)(bool);
  nsStaticLinkHelper(Func f) { f(true); }
};

/// \brief Helper struct to register the existence of statically linked plugins.
/// The macro NS_STATICLINK_LIBRARY will register a the given library name prepended with `ns` to the nsPlugin system.
/// Implemented in Plugin.cpp.
struct NS_FOUNDATION_DLL nsPluginRegister
{
  nsPluginRegister(const char* szName);
};

/// \brief The tool 'StaticLinkUtil' inserts this macro into each file in a library.
/// Each library also needs to contain exactly one instance of NS_STATICLINK_LIBRARY.
/// The macros create functions that reference each other, which means the linker is forced to look at all files in the library.
/// This in turn will drag all global variables into the visibility of the linker, and since it mustn't optimize them away,
/// they then end up in the final application, where they will do what they are meant for.
#  define NS_STATICLINK_FILE(LibraryName, UniqueName)        \
    extern "C"                                               \
    {                                                        \
      void nsReferenceFunction_##UniqueName(bool bReturn) {} \
      void nsReferenceFunction_##LibraryName(bool bReturn);  \
    }                                                        \
    static nsStaticLinkHelper StaticLinkHelper_##UniqueName(nsReferenceFunction_##LibraryName);

/// \brief Used by the tool 'StaticLinkUtil' to generate the block after NS_STATICLINK_LIBRARY, to create references to all
/// files inside a library. \see NS_STATICLINK_FILE
#  define NS_STATICLINK_REFERENCE(UniqueName)                   \
    void nsReferenceFunction_##UniqueName(bool bReturn = true); \
    nsReferenceFunction_##UniqueName()

/// \brief This must occur exactly once in each static library, such that all NS_STATICLINK_FILE macros can reference it.
#  define NS_STATICLINK_LIBRARY(LibraryName)                                                      \
    nsPluginRegister nsPluginRegister_##LibraryName(NS_PP_STRINGIFY(NS_CONCAT(ns, LibraryName))); \
    extern "C" void nsReferenceFunction_##LibraryName(bool bReturn = true)

#endif

namespace nsInternal
{
  template <typename T, size_t N>
  char (*ArraySizeHelper(T (&)[N]))[N];
}

/// \brief Macro to determine the size of a static array
#define NS_ARRAY_SIZE(a) (sizeof(*nsInternal::ArraySizeHelper(a)) + 0)

/// \brief Template helper which allows to suppress "Unused variable" warnings (e.g. result used in platform specific block, ..)
template <class T>
void NS_IGNORE_UNUSED(const T&)
{
}

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  define NS_DECL_EXPORT __declspec(dllexport)
#  define NS_DECL_IMPORT __declspec(dllimport)
#  define NS_DECL_EXPORT_FRIEND __declspec(dllexport)
#  define NS_DECL_IMPORT_FRIEND __declspec(dllimport)
#else
#  define NS_DECL_EXPORT [[gnu::visibility("default")]]
#  define NS_DECL_IMPORT [[gnu::visibility("default")]]
#  define NS_DECL_EXPORT_FRIEND
#  define NS_DECL_IMPORT_FRIEND
#endif

#if (__cplusplus >= 202002L || _MSVC_LANG >= 202002L)
#  undef NS_USE_CPP20_OPERATORS
#  define NS_USE_CPP20_OPERATORS NS_ON
#endif

#if NS_ENABLED(NS_USE_CPP20_OPERATORS)
// in C++ 20 we don't need to declare an operator!=, it is automatically generated from operator==
#  define NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(...) /*empty*/
#else
#  define NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(...)                                   \
    NS_ALWAYS_INLINE bool operator!=(NS_EXPAND_ARGS_COMMA(__VA_ARGS__) rhs) const \
    {                                                                             \
      return !(*this == rhs);                                                     \
    }
#endif
