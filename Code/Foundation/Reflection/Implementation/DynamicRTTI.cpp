#include <Foundation/FoundationPCH.h>

#include <Foundation/Reflection/Implementation/DynamicRTTI.h>
#include <Foundation/Reflection/Implementation/RTTI.h>

bool nsReflectedClass::IsInstanceOf(const nsRTTI* pType) const
{
  return GetDynamicRTTI()->IsDerivedFrom(pType);
}
