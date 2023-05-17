#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/Strings/StringBuilder.h>

class wdLogInterface;

/// \brief An wdResult with an additional message for the reason of failure
struct WD_FOUNDATION_DLL wdStatus
{
  WD_ALWAYS_INLINE explicit wdStatus()
    : m_Result(WD_FAILURE)
  {
  }

  // const char* version is needed for disambiguation
  explicit wdStatus(const char* szError)
    : m_Result(WD_FAILURE)
    , m_sMessage(szError)
  {
  }

  explicit wdStatus(wdResult r, wdStringView sError)
    : m_Result(r)
    , m_sMessage(sError)
  {
  }

  explicit wdStatus(wdStringView sError)
    : m_Result(WD_FAILURE)
    , m_sMessage(sError)
  {
  }

  WD_ALWAYS_INLINE wdStatus(wdResult r)
    : m_Result(r)
  {
  }

  explicit wdStatus(const wdFormatString& fmt);

  [[nodiscard]] WD_ALWAYS_INLINE bool Succeeded() const { return m_Result.Succeeded(); }
  [[nodiscard]] WD_ALWAYS_INLINE bool Failed() const { return m_Result.Failed(); }

  /// \brief Same as 'Succeeded()'.
  ///
  /// Allows wdStatus to be used in if statements:
  ///  - if (r)
  ///  - if (!r)
  ///  - if (r1 && r2)
  ///  - if (r1 || r2)
  ///
  /// Disallows anything else implicitly, e.g. all these won't compile:
  ///   - if (r == true)
  ///   - bool b = r;
  ///   - void* p = r;
  ///   - return r; // with bool return type
  explicit operator bool() const { return m_Result.Succeeded(); }

  /// \brief Special case to prevent this from working: "bool b = !r"
  wdResult operator!() const { return wdResult(m_Result.Succeeded() ? WD_FAILURE : WD_SUCCESS); }

  /// \brief If the state is WD_FAILURE, the message is written to the given log (or the currently active thread-local log).
  void LogFailure(wdLogInterface* pLog = nullptr);

  wdResult m_Result;
  wdString m_sMessage;
};

WD_ALWAYS_INLINE wdResult wdToResult(const wdStatus& result)
{
  return result.m_Result;
}
