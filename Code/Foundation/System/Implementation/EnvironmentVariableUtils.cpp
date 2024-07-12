#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/System/EnvironmentVariableUtils.h>
#include <Foundation/Threading/Mutex.h>
#include <Foundation/Utilities/ConversionUtils.h>

// The POSIX functions are not thread safe by definition.
static nsMutex s_EnvVarMutex;


nsString nsEnvironmentVariableUtils::GetValueString(nsStringView sName, nsStringView sDefault /*= nullptr*/)
{
  NS_ASSERT_DEV(!sName.IsEmpty(), "Null or empty name passed to nsEnvironmentVariableUtils::GetValueString()");

  NS_LOCK(s_EnvVarMutex);

  return GetValueStringImpl(sName, sDefault);
}

nsResult nsEnvironmentVariableUtils::SetValueString(nsStringView sName, nsStringView sValue)
{
  NS_LOCK(s_EnvVarMutex);

  return SetValueStringImpl(sName, sValue);
}

nsInt32 nsEnvironmentVariableUtils::GetValueInt(nsStringView sName, nsInt32 iDefault /*= -1*/)
{
  NS_LOCK(s_EnvVarMutex);

  nsString value = GetValueString(sName);

  if (value.IsEmpty())
    return iDefault;

  nsInt32 iRetVal = 0;
  if (nsConversionUtils::StringToInt(value, iRetVal).Succeeded())
    return iRetVal;
  else
    return iDefault;
}

nsResult nsEnvironmentVariableUtils::SetValueInt(nsStringView sName, nsInt32 iValue)
{
  nsStringBuilder sb;
  sb.SetFormat("{}", iValue);

  return SetValueString(sName, sb);
}

bool nsEnvironmentVariableUtils::IsVariableSet(nsStringView sName)
{
  NS_LOCK(s_EnvVarMutex);

  return IsVariableSetImpl(sName);
}

nsResult nsEnvironmentVariableUtils::UnsetVariable(nsStringView sName)
{
  NS_LOCK(s_EnvVarMutex);

  return UnsetVariableImpl(sName);
}
