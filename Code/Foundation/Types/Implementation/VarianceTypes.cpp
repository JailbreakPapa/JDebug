#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/Stream.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/VarianceTypes.h>
#include <Foundation/Types/VariantTypeRegistry.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsVarianceTypeBase, nsNoBase, 1, nsRTTINoAllocator)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Variance", m_fVariance)
  }
    NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsVarianceTypeFloat, nsVarianceTypeBase, 1, nsRTTIDefaultAllocator<nsVarianceTypeFloat>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Value", m_Value)
  }
    NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsVarianceTypeTime, nsVarianceTypeBase, 1, nsRTTIDefaultAllocator<nsVarianceTypeTime>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Value", m_Value)
  }
    NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsVarianceTypeAngle, nsVarianceTypeBase, 1, nsRTTIDefaultAllocator<nsVarianceTypeAngle>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Value", m_Value)
  }
    NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

NS_DEFINE_CUSTOM_VARIANT_TYPE(nsVarianceTypeFloat);
NS_DEFINE_CUSTOM_VARIANT_TYPE(nsVarianceTypeTime);
NS_DEFINE_CUSTOM_VARIANT_TYPE(nsVarianceTypeAngle);

NS_STATICLINK_FILE(Foundation, Foundation_Types_Implementation_VarianceTypes);
