#include <Foundation/FoundationInternal.h>
NS_FOUNDATION_INTERNAL_HEADER

#include <Foundation/System/EnvironmentVariableUtils.h>

nsString nsEnvironmentVariableUtils::GetValueStringImpl(nsStringView sName, nsStringView sDefault)
{
  NS_ASSERT_NOT_IMPLEMENTED
  return "";
}

nsResult nsEnvironmentVariableUtils::SetValueStringImpl(nsStringView sName, nsStringView szValue)
{
  NS_ASSERT_NOT_IMPLEMENTED
  return NS_FAILURE;
}

bool nsEnvironmentVariableUtils::IsVariableSetImpl(nsStringView sName)
{
  return false;
}

nsResult nsEnvironmentVariableUtils::UnsetVariableImpl(nsStringView sName)
{
  return NS_FAILURE;
}
