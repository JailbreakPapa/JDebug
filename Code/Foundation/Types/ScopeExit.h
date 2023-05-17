
#pragma once

#include <Foundation/Basics.h>

/// \file

/// \brief Macro to execute a piece of code when the current scope closes.
#define WD_SCOPE_EXIT(code) auto WD_CONCAT(scopeExit_, WD_SOURCE_LINE) = wdMakeScopeExit([&]() { code; })

/// \internal Helper class to implement WD_SCOPE_EXIT
template <typename T>
struct wdScopeExit
{
  WD_ALWAYS_INLINE wdScopeExit(T&& func)
    : m_func(std::forward<T>(func))
  {
  }

  WD_ALWAYS_INLINE ~wdScopeExit() { m_func(); }

  T m_func;
};

/// \internal Helper function to implement WD_SCOPE_EXIT
template <typename T>
WD_ALWAYS_INLINE wdScopeExit<T> wdMakeScopeExit(T&& func)
{
  return wdScopeExit<T>(std::forward<T>(func));
}
