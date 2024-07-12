#include <Foundation/FoundationInternal.h>
NS_FOUNDATION_INTERNAL_HEADER

#include <Foundation/System/EnvironmentVariableUtils.h>
#include <stdlib.h>

nsString nsEnvironmentVariableUtils::GetValueStringImpl(nsStringView sName, nsStringView sDefault)
{
  nsStringBuilder tmp;
  const char* value = getenv(sName.GetData(tmp));
  return value != nullptr ? value : sDefault;
}

nsResult nsEnvironmentVariableUtils::SetValueStringImpl(nsStringView sName, nsStringView sValue)
{
  nsStringBuilder tmp, tmp2;
  if (setenv(sName.GetData(tmp), sValue.GetData(tmp2), 1) == 0)
    return NS_SUCCESS;
  else
    return NS_FAILURE;
}

bool nsEnvironmentVariableUtils::IsVariableSetImpl(nsStringView sName)
{
  nsStringBuilder tmp;
  return getenv(sName.GetData(tmp)) != nullptr;
}

nsResult nsEnvironmentVariableUtils::UnsetVariableImpl(nsStringView sName)
{
  nsStringBuilder tmp;
  if (unsetenv(sName.GetData(tmp)) == 0)
    return NS_SUCCESS;
  else
    return NS_FAILURE;
}
