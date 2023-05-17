#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <stdlib.h>

wdString wdEnvironmentVariableUtils::GetValueStringImpl(const char* szName, const char* szDefault)
{
  const char* value = getenv(szName);
  return value != nullptr ? value : szDefault;
}

wdResult wdEnvironmentVariableUtils::SetValueStringImpl(const char* szName, const char* szValue)
{
  if (setenv(szName, szValue, 1) == 0)
    return WD_SUCCESS;
  else
    return WD_FAILURE;
}

bool wdEnvironmentVariableUtils::IsVariableSetImpl(const char* szName)
{
  return getenv(szName) != nullptr;
}

wdResult wdEnvironmentVariableUtils::UnsetVariableImpl(const char* szName)
{
  if (unsetenv(szName) == 0)
    return WD_SUCCESS;
  else
    return WD_FAILURE;
}
