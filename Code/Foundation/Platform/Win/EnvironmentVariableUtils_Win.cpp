#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)

#  include <Foundation/Logging/Log.h>
#  include <Foundation/Strings/StringBuilder.h>
#  include <Foundation/System/EnvironmentVariableUtils.h>
#  include <Foundation/Threading/Mutex.h>
#  include <Foundation/Utilities/ConversionUtils.h>

#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#  include <intsafe.h>

nsString nsEnvironmentVariableUtils::GetValueStringImpl(nsStringView sName, nsStringView szDefault)
{
  nsStringWChar szwName(sName);
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
    return nsString(szStaticValueBuffer);
  }
  // Static buffer was too small, do a heap allocation to query the value
  else if (res == ERANGE)
  {
    NS_ASSERT_DEV(uiRequiredSize != SIZE_T_MAX, "");
    const size_t uiDynamicSize = uiRequiredSize + 1;
    wchar_t* szDynamicBuffer = NS_DEFAULT_NEW_RAW_BUFFER(wchar_t, uiDynamicSize);
    nsMemoryUtils::ZeroFill(szDynamicBuffer, uiDynamicSize);

    res = _wgetenv_s(&uiRequiredSize, szDynamicBuffer, uiDynamicSize, szwName);

    if (res != 0)
    {
      nsLog::Error("Error getting environment variable \"{0}\" with dynamic buffer.", sName);
      NS_DEFAULT_DELETE_RAW_BUFFER(szDynamicBuffer);
      return szDefault;
    }
    else
    {
      nsString retVal(szDynamicBuffer);
      NS_DEFAULT_DELETE_RAW_BUFFER(szDynamicBuffer);
      return retVal;
    }
  }
  else
  {
    nsLog::Warning("Couldn't get environment variable value for \"{0}\", got {1} as a result.", sName, res);
    return szDefault;
  }
}

nsResult nsEnvironmentVariableUtils::SetValueStringImpl(nsStringView sName, nsStringView szValue)
{
  nsStringWChar szwName(sName);
  nsStringWChar szwValue(szValue);

  if (_wputenv_s(szwName, szwValue) == 0)
    return NS_SUCCESS;
  else
    return NS_FAILURE;
}

bool nsEnvironmentVariableUtils::IsVariableSetImpl(nsStringView sName)
{
  nsStringWChar szwName(sName);
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
    nsLog::Error("nsEnvironmentVariableUtils::IsVariableSet(\"{0}\") got {1} from _wgetenv_s.", sName, res);
    return false;
  }
}

nsResult nsEnvironmentVariableUtils::UnsetVariableImpl(nsStringView sName)
{
  nsStringWChar szwName(sName);

  if (_wputenv_s(szwName, L"") == 0)
    return NS_SUCCESS;
  else
    return NS_FAILURE;
}

#endif
