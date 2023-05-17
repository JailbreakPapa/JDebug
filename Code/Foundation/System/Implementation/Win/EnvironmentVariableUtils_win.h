#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#include <intsafe.h>

wdString wdEnvironmentVariableUtils::GetValueStringImpl(const char* szName, const char* szDefault)
{
  wdStringWChar szwName(szName);
  wchar_t szStaticValueBuffer[64] = {0};
  size_t uiRequiredSize = 0;

  errno_t res = _wgetenv_s(&uiRequiredSize, szStaticValueBuffer, szwName);

  // Variable doesn't exist
  if (uiRequiredSize == 0)
  {
    return szDefault;
  }

  // Succeeded
  if (res == 0)
  {
    return wdString(szStaticValueBuffer);
  }
  // Static buffer was too small, do a heap allocation to query the value
  else if (res == ERANGE)
  {
    WD_ASSERT_DEV(uiRequiredSize != SIZE_T_MAX, "");
    const size_t uiDynamicSize = uiRequiredSize + 1;
    wchar_t* szDynamicBuffer = WD_DEFAULT_NEW_RAW_BUFFER(wchar_t, uiDynamicSize);
    wdMemoryUtils::ZeroFill(szDynamicBuffer, uiDynamicSize);

    res = _wgetenv_s(&uiRequiredSize, szDynamicBuffer, uiDynamicSize, szwName);

    if (res != 0)
    {
      wdLog::Error("Error getting environment variable \"{0}\" with dynamic buffer.", szName);
      WD_DEFAULT_DELETE_RAW_BUFFER(szDynamicBuffer);
      return szDefault;
    }
    else
    {
      wdString retVal(szDynamicBuffer);
      WD_DEFAULT_DELETE_RAW_BUFFER(szDynamicBuffer);
      return retVal;
    }
  }
  else
  {
    wdLog::Warning("Couldn't get environment variable value for \"{0}\", got {1} as a result.", szName, res);
    return szDefault;
  }
}

wdResult wdEnvironmentVariableUtils::SetValueStringImpl(const char* szName, const char* szValue)
{
  wdStringWChar szwName(szName);
  wdStringWChar szwValue(szValue);

  if (_wputenv_s(szwName, szwValue) == 0)
    return WD_SUCCESS;
  else
    return WD_FAILURE;
}

bool wdEnvironmentVariableUtils::IsVariableSetImpl(const char* szName)
{
  wdStringWChar szwName(szName);
  wchar_t szStaticValueBuffer[16] = {0};
  size_t uiRequiredSize = 0;

  errno_t res = _wgetenv_s(&uiRequiredSize, szStaticValueBuffer, szwName);

  if (res == 0 || res == ERANGE)
  {
    // Variable doesn't exist if uiRequiredSize is 0
    return uiRequiredSize > 0;
  }
  else
  {
    wdLog::Error("wdEnvironmentVariableUtils::IsVariableSet(\"{0}\") got {1} from _wgetenv_s.", szName, res);
    return false;
  }
}

wdResult wdEnvironmentVariableUtils::UnsetVariableImpl(const char* szName)
{
  wdStringWChar szwName(szName);

  if (_wputenv_s(szwName, L"") == 0)
    return WD_SUCCESS;
  else
    return WD_FAILURE;
}
