#include <Foundation/FoundationPCH.h>

#include <Foundation/Reflection/Implementation/DynamicRTTI.h>
#include <Foundation/Reflection/Implementation/RTTI.h>

bool wdReflectedClass::IsInstanceOf(const wdRTTI* pType) const
{
  return GetDynamicRTTI()->IsDerivedFrom(pType);
}


WD_STATICLINK_FILE(Foundation, Foundation_Reflection_Implementation_DynamicRTTI);
