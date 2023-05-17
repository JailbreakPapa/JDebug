#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

wdString wdEnvironmentVariableUtils::GetValueStringImpl(const char* szName, const char* szDefault)
{
  WD_ASSERT_NOT_IMPLEMENTED
  return "";
}

wdResult wdEnvironmentVariableUtils::SetValueStringImpl(const char* szName, const char* szValue)
{
  WD_ASSERT_NOT_IMPLEMENTED
  return WD_FAILURE;
}

bool wdEnvironmentVariableUtils::IsVariableSetImpl(const char* szName)
{
  return false;
}

wdResult wdEnvironmentVariableUtils::UnsetVariableImpl(const char* szName)
{
  return WD_FAILURE;
}
