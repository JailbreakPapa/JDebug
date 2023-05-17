#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/Stream.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/VarianceTypes.h>
#include <Foundation/Types/VariantTypeRegistry.h>

// clang-format off
WD_BEGIN_STATIC_REFLECTED_TYPE(wdVarianceTypeBase, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Variance", m_fVariance)
  }
    WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdVarianceTypeFloat, wdVarianceTypeBase, 1, wdRTTIDefaultAllocator<wdVarianceTypeFloat>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Value", m_Value)
  }
    WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdVarianceTypeTime, wdVarianceTypeBase, 1, wdRTTIDefaultAllocator<wdVarianceTypeTime>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Value", m_Value)
  }
    WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdVarianceTypeAngle, wdVarianceTypeBase, 1, wdRTTIDefaultAllocator<wdVarianceTypeAngle>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Value", m_Value)
  }
    WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;
// clang-format on

WD_DEFINE_CUSTOM_VARIANT_TYPE(wdVarianceTypeFloat);
WD_DEFINE_CUSTOM_VARIANT_TYPE(wdVarianceTypeTime);
WD_DEFINE_CUSTOM_VARIANT_TYPE(wdVarianceTypeAngle);

WD_STATICLINK_FILE(Foundation, Foundation_Types_Implementation_VarianceTypes);
