#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Basics/Platform/Win/MinWindows.h>
#include <Foundation/Strings/String.h>

/// Build string implementation for HRESULT.
WD_FOUNDATION_DLL wdString wdHRESULTtoString(wdMinWindows::HRESULT result);

/// Conversion of HRESULT to wdResult.
WD_ALWAYS_INLINE wdResult wdToResult(wdMinWindows::HRESULT result)
{
  return result >= 0 ? WD_SUCCESS : WD_FAILURE;
}

#define WD_HRESULT_TO_FAILURE(code)   \
  do                                  \
  {                                   \
    wdMinWindows::HRESULT s = (code); \
    if (s < 0)                        \
      return WD_FAILURE;              \
  } while (false)

#define WD_HRESULT_TO_FAILURE_LOG(code)                                                      \
  do                                                                                         \
  {                                                                                          \
    wdMinWindows::HRESULT s = (code);                                                        \
    if (s < 0)                                                                               \
    {                                                                                        \
      wdLog::Error("Call '{0}' failed with: {1}", WD_STRINGIZE(code), wdHRESULTtoString(s)); \
      return WD_FAILURE;                                                                     \
    }                                                                                        \
  } while (false)

#define WD_HRESULT_TO_LOG(code)                                                              \
  do                                                                                         \
  {                                                                                          \
    wdMinWindows::HRESULT s = (code);                                                        \
    if (s < 0)                                                                               \
    {                                                                                        \
      wdLog::Error("Call '{0}' failed with: {1}", WD_STRINGIZE(code), wdHRESULTtoString(s)); \
    }                                                                                        \
  } while (false)

#define WD_NO_RETURNVALUE

#define WD_HRESULT_TO_LOG_RET(code, ret)                                                     \
  do                                                                                         \
  {                                                                                          \
    wdMinWindows::HRESULT s = (code);                                                        \
    if (s < 0)                                                                               \
    {                                                                                        \
      wdLog::Error("Call '{0}' failed with: {1}", WD_STRINGIZE(code), wdHRESULTtoString(s)); \
      return ret;                                                                            \
    }                                                                                        \
  } while (false)

#define WD_HRESULT_TO_ASSERT(code)                                                                  \
  do                                                                                                \
  {                                                                                                 \
    wdMinWindows::HRESULT s = (code);                                                               \
    WD_ASSERT_DEV(s >= 0, "Call '{0}' failed with: {1}", WD_STRINGIZE(code), wdHRESULTtoString(s)); \
  } while (false)
