#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Basics/Platform/Win/MinWindows.h>
#include <Foundation/Strings/String.h>

/// Build string implementation for HRESULT.
NS_FOUNDATION_DLL nsString nsHRESULTtoString(nsMinWindows::HRESULT result);

/// Conversion of HRESULT to nsResult.
NS_ALWAYS_INLINE nsResult nsToResult(nsMinWindows::HRESULT result)
{
  return result >= 0 ? NS_SUCCESS : NS_FAILURE;
}

#define NS_HRESULT_TO_FAILURE(code)   \
  do                                  \
  {                                   \
    nsMinWindows::HRESULT s = (code); \
    if (s < 0)                        \
      return NS_FAILURE;              \
  } while (false)

#define NS_HRESULT_TO_FAILURE_LOG(code)                                                      \
  do                                                                                         \
  {                                                                                          \
    nsMinWindows::HRESULT s = (code);                                                        \
    if (s < 0)                                                                               \
    {                                                                                        \
      nsLog::Error("Call '{0}' failed with: {1}", NS_STRINGIZE(code), nsHRESULTtoString(s)); \
      return NS_FAILURE;                                                                     \
    }                                                                                        \
  } while (false)

#define NS_HRESULT_TO_LOG(code)                                                              \
  do                                                                                         \
  {                                                                                          \
    nsMinWindows::HRESULT s = (code);                                                        \
    if (s < 0)                                                                               \
    {                                                                                        \
      nsLog::Error("Call '{0}' failed with: {1}", NS_STRINGIZE(code), nsHRESULTtoString(s)); \
    }                                                                                        \
  } while (false)

#define NS_NO_RETURNVALUE

#define NS_HRESULT_TO_LOG_RET(code, ret)                                                     \
  do                                                                                         \
  {                                                                                          \
    nsMinWindows::HRESULT s = (code);                                                        \
    if (s < 0)                                                                               \
    {                                                                                        \
      nsLog::Error("Call '{0}' failed with: {1}", NS_STRINGIZE(code), nsHRESULTtoString(s)); \
      return ret;                                                                            \
    }                                                                                        \
  } while (false)

#define NS_HRESULT_TO_ASSERT(code)                                                                  \
  do                                                                                                \
  {                                                                                                 \
    nsMinWindows::HRESULT s = (code);                                                               \
    NS_ASSERT_DEV(s >= 0, "Call '{0}' failed with: {1}", NS_STRINGIZE(code), nsHRESULTtoString(s)); \
  } while (false)
