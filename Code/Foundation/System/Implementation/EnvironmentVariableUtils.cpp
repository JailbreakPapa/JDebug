#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/System/EnvironmentVariableUtils.h>
#include <Foundation/Threading/Mutex.h>
#include <Foundation/Utilities/ConversionUtils.h>

// The POSIX functions are not thread safe by definition.
static wdMutex s_EnvVarMutex;


wdString wdEnvironmentVariableUtils::GetValueString(const char* szName, const char* szDefault /*= nullptr*/)
{
  WD_ASSERT_DEV(!wdStringUtils::IsNullOrEmpty(szName), "Null or empty name passed to wdEnvironmentVariableUtils::GetValueString()");

  WD_LOCK(s_EnvVarMutex);

  return GetValueStringImpl(szName, szDefault);
}

wdResult wdEnvironmentVariableUtils::SetValueString(const char* szName, const char* szValue)
{
  WD_LOCK(s_EnvVarMutex);

  return SetValueStringImpl(szName, szValue);
}

wdInt32 wdEnvironmentVariableUtils::GetValueInt(const char* szName, wdInt32 iDefault /*= -1*/)
{
  WD_LOCK(s_EnvVarMutex);

  wdString value = GetValueString(szName);

  if (value.IsEmpty())
    return iDefault;

  wdInt32 iRetVal = 0;
  if (wdConversionUtils::StringToInt(value, iRetVal).Succeeded())
    return iRetVal;
  else
    return iDefault;
}

wdResult wdEnvironmentVariableUtils::SetValueInt(const char* szName, wdInt32 iValue)
{
  wdStringBuilder sb;
  sb.Format("{}", iValue);

  return SetValueString(szName, sb);
}

bool wdEnvironmentVariableUtils::IsVariableSet(const char* szName)
{
  WD_LOCK(s_EnvVarMutex);

  return IsVariableSetImpl(szName);
}

wdResult wdEnvironmentVariableUtils::UnsetVariable(const char* szName)
{
  WD_LOCK(s_EnvVarMutex);

  return UnsetVariableImpl(szName);
}

#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
#  include <Foundation/System/Implementation/Win/EnvironmentVariableUtils_win.h>
#elif WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
#  include <Foundation/System/Implementation/Win/EnvironmentVariableUtils_win_uwp.h>
#else
#  include <Foundation/System/Implementation/Posix/EnvironmentVariableUtils_posix.h>
#endif


WD_STATICLINK_FILE(Foundation, Foundation_System_Implementation_EnvironmentVariableUtils);
