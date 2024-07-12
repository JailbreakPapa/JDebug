#include <Foundation/FoundationPCH.h>

#include <Foundation/Reflection/Reflection.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsPropertyAttribute, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsReadOnlyAttribute, 1, nsRTTIDefaultAllocator<nsReadOnlyAttribute>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsHiddenAttribute, 1, nsRTTIDefaultAllocator<nsHiddenAttribute>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsTemporaryAttribute, 1, nsRTTIDefaultAllocator<nsTemporaryAttribute>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_BITFLAGS(nsDependencyFlags, 1)
  NS_BITFLAGS_CONSTANTS(nsDependencyFlags::Package, nsDependencyFlags::Thumbnail, nsDependencyFlags::Transform)
NS_END_STATIC_REFLECTED_BITFLAGS;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCategoryAttribute, 1, nsRTTIDefaultAllocator<nsCategoryAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Category", m_sCategory),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsInDevelopmentAttribute, 1, nsRTTIDefaultAllocator<nsInDevelopmentAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Phase", m_Phase),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(nsInt32),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;


const char* nsInDevelopmentAttribute::GetString() const
{
  switch (m_Phase)
  {
  case Phase::Alpha:
    return "ALPHA";

  case Phase::Beta:
    return "BETA";

    NS_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  return "";
}

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsTitleAttribute, 1, nsRTTIDefaultAllocator<nsTitleAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Title", m_sTitle),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsColorAttribute, 1, nsRTTIDefaultAllocator<nsColorAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Color", m_Color),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(nsColor),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsExposeColorAlphaAttribute, 1, nsRTTIDefaultAllocator<nsExposeColorAlphaAttribute>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSuffixAttribute, 1, nsRTTIDefaultAllocator<nsSuffixAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Suffix", m_sSuffix),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMinValueTextAttribute, 1, nsRTTIDefaultAllocator<nsMinValueTextAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Text", m_sText),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDefaultValueAttribute, 1, nsRTTIDefaultAllocator<nsDefaultValueAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Value", m_Value),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const nsVariant&),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsImageSliderUiAttribute, 1, nsRTTIDefaultAllocator<nsImageSliderUiAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ImageGenerator", m_sImageGenerator),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsClampValueAttribute, 1, nsRTTIDefaultAllocator<nsClampValueAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Min", m_MinValue),
    NS_MEMBER_PROPERTY("Max", m_MaxValue),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const nsVariant&, const nsVariant&),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsGroupAttribute, 1, nsRTTIDefaultAllocator<nsGroupAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Group", m_sGroup),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*, float),
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*, float),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

nsGroupAttribute::nsGroupAttribute()
= default;

nsGroupAttribute::nsGroupAttribute(const char* szGroup, float fOrder)
  : m_sGroup(szGroup)
  , m_fOrder(fOrder)
{
}

nsGroupAttribute::nsGroupAttribute(const char* szGroup, const char* szIconName, float fOrder)
  : m_sGroup(szGroup)
  , m_sIconName(szIconName)
  , m_fOrder(fOrder)
{
}

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsTypeWidgetAttribute, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsContainerWidgetAttribute, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsTagSetWidgetAttribute, 1, nsRTTIDefaultAllocator<nsTagSetWidgetAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Filter", m_sTagFilter),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsNoTemporaryTransactionsAttribute, 1, nsRTTIDefaultAllocator<nsNoTemporaryTransactionsAttribute>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsExposedParametersAttribute, 1, nsRTTIDefaultAllocator<nsExposedParametersAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ParametersSource", m_sParametersSource),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDynamicDefaultValueAttribute, 1, nsRTTIDefaultAllocator<nsDynamicDefaultValueAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ClassSource", m_sClassSource),
    NS_MEMBER_PROPERTY("ClassType", m_sClassType),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsContainerAttribute, 1, nsRTTIDefaultAllocator<nsContainerAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("CanAdd", m_bCanAdd),
    NS_MEMBER_PROPERTY("CanDelete", m_bCanDelete),
    NS_MEMBER_PROPERTY("CanMove", m_bCanMove),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(bool, bool, bool),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsFileBrowserAttribute, 1, nsRTTIDefaultAllocator<nsFileBrowserAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Title", m_sDialogTitle),
    NS_MEMBER_PROPERTY("Filter", m_sTypeFilter),
    NS_MEMBER_PROPERTY("CustomAction", m_sCustomAction),
    NS_BITFLAGS_MEMBER_PROPERTY("DependencyFlags", nsDependencyFlags, m_DependencyFlags),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(nsStringView, nsStringView),
    NS_CONSTRUCTOR_PROPERTY(nsStringView, nsStringView, nsStringView),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsExternalFileBrowserAttribute, 1, nsRTTIDefaultAllocator<nsExternalFileBrowserAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Title", m_sDialogTitle),
    NS_MEMBER_PROPERTY("Filter", m_sTypeFilter),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(nsStringView, nsStringView),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAssetBrowserAttribute, 1, nsRTTIDefaultAllocator<nsAssetBrowserAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Filter", m_sTypeFilter),
    NS_MEMBER_PROPERTY("RequiredTag", m_sRequiredTag),
    NS_BITFLAGS_MEMBER_PROPERTY("DependencyFlags", nsDependencyFlags, m_DependencyFlags),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*),
    NS_CONSTRUCTOR_PROPERTY(const char*, nsBitflags<nsDependencyFlags>),
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*),
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*, nsBitflags<nsDependencyFlags>),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDynamicEnumAttribute, 1, nsRTTIDefaultAllocator<nsDynamicEnumAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
   NS_MEMBER_PROPERTY("DynamicEnum", m_sDynamicEnumName),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
   NS_CONSTRUCTOR_PROPERTY(const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDynamicStringEnumAttribute, 1, nsRTTIDefaultAllocator<nsDynamicStringEnumAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("DynamicEnum", m_sDynamicEnumName),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDynamicBitflagsAttribute, 1, nsRTTIDefaultAllocator<nsDynamicBitflagsAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
   NS_MEMBER_PROPERTY("DynamicBitflags", m_sDynamicBitflagsName),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
   NS_CONSTRUCTOR_PROPERTY(nsStringView),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsManipulatorAttribute, 1, nsRTTINoAllocator)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Property1", m_sProperty1),
    NS_MEMBER_PROPERTY("Property2", m_sProperty2),
    NS_MEMBER_PROPERTY("Property3", m_sProperty3),
    NS_MEMBER_PROPERTY("Property4", m_sProperty4),
    NS_MEMBER_PROPERTY("Property5", m_sProperty5),
    NS_MEMBER_PROPERTY("Property6", m_sProperty6),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsManipulatorAttribute::nsManipulatorAttribute(const char* szProperty1, const char* szProperty2 /*= nullptr*/, const char* szProperty3 /*= nullptr*/,
  const char* szProperty4 /*= nullptr*/, const char* szProperty5 /*= nullptr*/, const char* szProperty6 /*= nullptr*/)
  : m_sProperty1(szProperty1)
  , m_sProperty2(szProperty2)
  , m_sProperty3(szProperty3)
  , m_sProperty4(szProperty4)
  , m_sProperty5(szProperty5)
  , m_sProperty6(szProperty6)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSphereManipulatorAttribute, 1, nsRTTIDefaultAllocator<nsSphereManipulatorAttribute>)
{
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*),
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsSphereManipulatorAttribute::nsSphereManipulatorAttribute()
  : nsManipulatorAttribute(nullptr)
{
}

nsSphereManipulatorAttribute::nsSphereManipulatorAttribute(const char* szOuterRadius, const char* szInnerRadius)
  : nsManipulatorAttribute(szOuterRadius, szInnerRadius)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCapsuleManipulatorAttribute, 1, nsRTTIDefaultAllocator<nsCapsuleManipulatorAttribute>)
{
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsCapsuleManipulatorAttribute::nsCapsuleManipulatorAttribute()
  : nsManipulatorAttribute(nullptr)
{
}

nsCapsuleManipulatorAttribute::nsCapsuleManipulatorAttribute(const char* szLength, const char* szRadius)
  : nsManipulatorAttribute(szLength, szRadius)
{
}


//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsBoxManipulatorAttribute, 1, nsRTTIDefaultAllocator<nsBoxManipulatorAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("scale", m_fSizeScale),
    NS_MEMBER_PROPERTY("recenter", m_bRecenterParent),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*, bool, float),
    NS_CONSTRUCTOR_PROPERTY(const char*, bool, float),
    NS_CONSTRUCTOR_PROPERTY(const char*, bool, float, const char*),
    NS_CONSTRUCTOR_PROPERTY(const char*, bool, float, const char*, const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsBoxManipulatorAttribute::nsBoxManipulatorAttribute()
  : nsManipulatorAttribute(nullptr)
{
}

nsBoxManipulatorAttribute::nsBoxManipulatorAttribute(const char* szSizeProperty, float fSizeScale, bool bRecenterParent, const char* szOffsetProperty, const char* szRotationProperty)
  : nsManipulatorAttribute(szSizeProperty, szOffsetProperty, szRotationProperty)
{
  m_bRecenterParent = bRecenterParent;
  m_fSizeScale = fSizeScale;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsNonUniformBoxManipulatorAttribute, 1, nsRTTIDefaultAllocator<nsNonUniformBoxManipulatorAttribute>)
{
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*),
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*, const char*, const char*, const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsNonUniformBoxManipulatorAttribute::nsNonUniformBoxManipulatorAttribute()
  : nsManipulatorAttribute(nullptr)
{
}

nsNonUniformBoxManipulatorAttribute::nsNonUniformBoxManipulatorAttribute(
  const char* szNegXProp, const char* szPosXProp, const char* szNegYProp, const char* szPosYProp, const char* szNegZProp, const char* szPosZProp)
  : nsManipulatorAttribute(szNegXProp, szPosXProp, szNegYProp, szPosYProp, szNegZProp, szPosZProp)
{
}

nsNonUniformBoxManipulatorAttribute::nsNonUniformBoxManipulatorAttribute(const char* szSizeX, const char* szSizeY, const char* szSizeZ)
  : nsManipulatorAttribute(szSizeX, szSizeY, szSizeZ)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsConeLengthManipulatorAttribute, 1, nsRTTIDefaultAllocator<nsConeLengthManipulatorAttribute>)
{
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsConeLengthManipulatorAttribute::nsConeLengthManipulatorAttribute()
  : nsManipulatorAttribute(nullptr)
{
}

nsConeLengthManipulatorAttribute::nsConeLengthManipulatorAttribute(const char* szRadiusProperty)
  : nsManipulatorAttribute(szRadiusProperty)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsConeAngleManipulatorAttribute, 1, nsRTTIDefaultAllocator<nsConeAngleManipulatorAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("scale", m_fScale),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*),
    NS_CONSTRUCTOR_PROPERTY(const char*, float),
    NS_CONSTRUCTOR_PROPERTY(const char*, float, const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsConeAngleManipulatorAttribute::nsConeAngleManipulatorAttribute()
  : nsManipulatorAttribute(nullptr)
{
  m_fScale = 1.0f;
}

nsConeAngleManipulatorAttribute::nsConeAngleManipulatorAttribute(const char* szAngleProperty, float fScale, const char* szRadiusProperty)
  : nsManipulatorAttribute(szAngleProperty, szRadiusProperty)
{
  m_fScale = fScale;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsTransformManipulatorAttribute, 1, nsRTTIDefaultAllocator<nsTransformManipulatorAttribute>)
{
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsTransformManipulatorAttribute::nsTransformManipulatorAttribute()
  : nsManipulatorAttribute(nullptr)
{
}

nsTransformManipulatorAttribute::nsTransformManipulatorAttribute(
  const char* szTranslateProperty, const char* szRotateProperty, const char* szScaleProperty, const char* szOffsetTranslation, const char* szOffsetRotation)
  : nsManipulatorAttribute(szTranslateProperty, szRotateProperty, szScaleProperty, szOffsetTranslation, szOffsetRotation)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsBoneManipulatorAttribute, 1, nsRTTIDefaultAllocator<nsBoneManipulatorAttribute>)
{
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsBoneManipulatorAttribute::nsBoneManipulatorAttribute()
  : nsManipulatorAttribute(nullptr)
{
}

nsBoneManipulatorAttribute::nsBoneManipulatorAttribute(const char* szTransformProperty, const char* szBindTo)
  : nsManipulatorAttribute(szTransformProperty, szBindTo)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_STATIC_REFLECTED_BITFLAGS(nsVisualizerAnchor, 1)
  NS_BITFLAGS_CONSTANTS(nsVisualizerAnchor::Center, nsVisualizerAnchor::PosX, nsVisualizerAnchor::NegX, nsVisualizerAnchor::PosY, nsVisualizerAnchor::NegY, nsVisualizerAnchor::PosZ, nsVisualizerAnchor::NegZ)
NS_END_STATIC_REFLECTED_BITFLAGS;
// clang-format on

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsVisualizerAttribute, 1, nsRTTINoAllocator)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Property1", m_sProperty1),
    NS_MEMBER_PROPERTY("Property2", m_sProperty2),
    NS_MEMBER_PROPERTY("Property3", m_sProperty3),
    NS_MEMBER_PROPERTY("Property4", m_sProperty4),
    NS_MEMBER_PROPERTY("Property5", m_sProperty5),
    NS_BITFLAGS_MEMBER_PROPERTY("Anchor", nsVisualizerAnchor, m_Anchor),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsVisualizerAttribute::nsVisualizerAttribute(const char* szProperty1, const char* szProperty2 /*= nullptr*/, const char* szProperty3 /*= nullptr*/,
  const char* szProperty4 /*= nullptr*/, const char* szProperty5 /*= nullptr*/)
  : m_sProperty1(szProperty1)
  , m_sProperty2(szProperty2)
  , m_sProperty3(szProperty3)
  , m_sProperty4(szProperty4)
  , m_sProperty5(szProperty5)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsBoxVisualizerAttribute, 1, nsRTTIDefaultAllocator<nsBoxVisualizerAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Color", m_Color),
    NS_MEMBER_PROPERTY("OffsetOrScale", m_vOffsetOrScale),
    NS_MEMBER_PROPERTY("SizeScale", m_fSizeScale),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*, float, const nsColor&, const char*, nsBitflags<nsVisualizerAnchor>, nsVec3, const char*, const char*),
    NS_CONSTRUCTOR_PROPERTY(const char*, float, const nsColor&, const char*, nsBitflags<nsVisualizerAnchor>, nsVec3, const char*),
    NS_CONSTRUCTOR_PROPERTY(const char*, float, const nsColor&, const char*, nsBitflags<nsVisualizerAnchor>, nsVec3),
    NS_CONSTRUCTOR_PROPERTY(const char*, float, const nsColor&, const char*, nsBitflags<nsVisualizerAnchor>),
    NS_CONSTRUCTOR_PROPERTY(const char*, float, const nsColor&, const char*),
    NS_CONSTRUCTOR_PROPERTY(const char*, float, const nsColor&),
    NS_CONSTRUCTOR_PROPERTY(const char*, float),
    NS_CONSTRUCTOR_PROPERTY(const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsBoxVisualizerAttribute::nsBoxVisualizerAttribute()
  : nsVisualizerAttribute(nullptr)
{
}

nsBoxVisualizerAttribute::nsBoxVisualizerAttribute(const char* szSizeProperty, float fSizeScale, const nsColor& fixedColor /*= nsColorScheme::LightUI(nsColorScheme::Grape)*/, const char* szColorProperty /*= nullptr*/, nsBitflags<nsVisualizerAnchor> anchor /*= nsVisualizerAnchor::Center*/, nsVec3 vOffsetOrScale /*= nsVec3::MakeZero*/, const char* szOffsetProperty /*= nullptr*/, const char* szRotationProperty /*= nullptr*/)
  : nsVisualizerAttribute(szSizeProperty, szColorProperty, szOffsetProperty, szRotationProperty)
  , m_Color(fixedColor)
  , m_vOffsetOrScale(vOffsetOrScale)
{
  m_Anchor = anchor;
  m_fSizeScale = fSizeScale;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSphereVisualizerAttribute, 1, nsRTTIDefaultAllocator<nsSphereVisualizerAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Color", m_Color),
    NS_MEMBER_PROPERTY("OffsetOrScale", m_vOffsetOrScale),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*, const nsColor&, const char*, nsBitflags<nsVisualizerAnchor>, nsVec3, const char*),
    NS_CONSTRUCTOR_PROPERTY(const char*, const nsColor&, const char*, nsBitflags<nsVisualizerAnchor>, nsVec3),
    NS_CONSTRUCTOR_PROPERTY(const char*, const nsColor&, const char*, nsBitflags<nsVisualizerAnchor>),
    NS_CONSTRUCTOR_PROPERTY(const char*, const nsColor&, const char*),
    NS_CONSTRUCTOR_PROPERTY(const char*, const nsColor&),
    NS_CONSTRUCTOR_PROPERTY(const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsSphereVisualizerAttribute::nsSphereVisualizerAttribute()
  : nsVisualizerAttribute(nullptr)
{
}

nsSphereVisualizerAttribute::nsSphereVisualizerAttribute(const char* szRadiusProperty, const nsColor& fixedColor /*= nsColorScheme::LightUI(nsColorScheme::Grape)*/, const char* szColorProperty /*= nullptr*/, nsBitflags<nsVisualizerAnchor> anchor /*= nsVisualizerAnchor::Center*/, nsVec3 vOffsetOrScale /*= nsVec3::MakeZero*/, const char* szOffsetProperty /*= nullptr*/)
  : nsVisualizerAttribute(szRadiusProperty, szColorProperty, szOffsetProperty)
  , m_Color(fixedColor)
  , m_vOffsetOrScale(vOffsetOrScale)
{
  m_Anchor = anchor;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCapsuleVisualizerAttribute, 1, nsRTTIDefaultAllocator<nsCapsuleVisualizerAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Color", m_Color),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*, const nsColor&, const char*, nsBitflags<nsVisualizerAnchor>),
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*, const nsColor&, const char*),
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*, const nsColor&),
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsCapsuleVisualizerAttribute::nsCapsuleVisualizerAttribute()
  : nsVisualizerAttribute(nullptr)
{
}

nsCapsuleVisualizerAttribute::nsCapsuleVisualizerAttribute(const char* szHeightProperty, const char* szRadiusProperty, const nsColor& fixedColor /*= nsColorScheme::LightUI(nsColorScheme::Grape)*/, const char* szColorProperty /*= nullptr*/, nsBitflags<nsVisualizerAnchor> anchor /*= nsVisualizerAnchor::Center*/)
  : nsVisualizerAttribute(szHeightProperty, szRadiusProperty, szColorProperty)
  , m_Color(fixedColor)
{
  m_Anchor = anchor;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCylinderVisualizerAttribute, 1, nsRTTIDefaultAllocator<nsCylinderVisualizerAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Color", m_Color),
    NS_MEMBER_PROPERTY("OffsetOrScale", m_vOffsetOrScale),
    NS_ENUM_MEMBER_PROPERTY("Axis", nsBasisAxis, m_Axis),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(nsEnum<nsBasisAxis>, const char*, const char*, const nsColor&, const char*, nsBitflags<nsVisualizerAnchor>, nsVec3, const char*),
    NS_CONSTRUCTOR_PROPERTY(nsEnum<nsBasisAxis>, const char*, const char*, const nsColor&, const char*, nsBitflags<nsVisualizerAnchor>, nsVec3),
    NS_CONSTRUCTOR_PROPERTY(nsEnum<nsBasisAxis>, const char*, const char*, const nsColor&, const char*, nsBitflags<nsVisualizerAnchor>),
    NS_CONSTRUCTOR_PROPERTY(nsEnum<nsBasisAxis>, const char*, const char*, const nsColor&, const char*),
    NS_CONSTRUCTOR_PROPERTY(nsEnum<nsBasisAxis>, const char*, const char*, const nsColor&),
    NS_CONSTRUCTOR_PROPERTY(nsEnum<nsBasisAxis>, const char*, const char*),
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*, const nsColor&, const char*, nsBitflags<nsVisualizerAnchor>, nsVec3, const char*),
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*, const nsColor&, const char*, nsBitflags<nsVisualizerAnchor>, nsVec3),
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*, const nsColor&, const char*, nsBitflags<nsVisualizerAnchor>),
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*, const nsColor&, const char*),
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*, const nsColor&),
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsCylinderVisualizerAttribute::nsCylinderVisualizerAttribute()
  : nsVisualizerAttribute(nullptr)
{
}

nsCylinderVisualizerAttribute::nsCylinderVisualizerAttribute(nsEnum<nsBasisAxis> axis, const char* szHeightProperty, const char* szRadiusProperty, const nsColor& fixedColor /*= nsColorScheme::LightUI(nsColorScheme::Grape)*/, const char* szColorProperty /*= nullptr*/, nsBitflags<nsVisualizerAnchor> anchor /*= nsVisualizerAnchor::Center*/, nsVec3 vOffsetOrScale /*= nsVec3::MakeZero*/, const char* szOffsetProperty /*= nullptr*/)
  : nsVisualizerAttribute(szHeightProperty, szRadiusProperty, szColorProperty, szOffsetProperty)
  , m_Color(fixedColor)
  , m_vOffsetOrScale(vOffsetOrScale)
  , m_Axis(axis)
{
  m_Anchor = anchor;
}

nsCylinderVisualizerAttribute::nsCylinderVisualizerAttribute(const char* szAxisProperty, const char* szHeightProperty, const char* szRadiusProperty, const nsColor& fixedColor /*= nsColorScheme::LightUI(nsColorScheme::Grape)*/, const char* szColorProperty /*= nullptr*/, nsBitflags<nsVisualizerAnchor> anchor /*= nsVisualizerAnchor::Center*/, nsVec3 vOffsetOrScale /*= nsVec3::MakeZero()*/, const char* szOffsetProperty /*= nullptr*/)
  : nsVisualizerAttribute(szHeightProperty, szRadiusProperty, szColorProperty, szOffsetProperty, szAxisProperty)
  , m_Color(fixedColor)
  , m_vOffsetOrScale(vOffsetOrScale)
{
  m_Axis = nsBasisAxis::Default;
  m_Anchor = anchor;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDirectionVisualizerAttribute, 1, nsRTTIDefaultAllocator<nsDirectionVisualizerAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ENUM_MEMBER_PROPERTY("Axis", nsBasisAxis, m_Axis),
    NS_MEMBER_PROPERTY("Color", m_Color),
    NS_MEMBER_PROPERTY("Scale", m_fScale)
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(nsEnum<nsBasisAxis>, float, const nsColor&, const char*, const char*),
    NS_CONSTRUCTOR_PROPERTY(nsEnum<nsBasisAxis>, float, const nsColor&, const char*),
    NS_CONSTRUCTOR_PROPERTY(nsEnum<nsBasisAxis>, float, const nsColor&),
    NS_CONSTRUCTOR_PROPERTY(nsEnum<nsBasisAxis>, float),
    NS_CONSTRUCTOR_PROPERTY(const char*, float, const nsColor&, const char*, const char*),
    NS_CONSTRUCTOR_PROPERTY(const char*, float, const nsColor&, const char*),
    NS_CONSTRUCTOR_PROPERTY(const char*, float, const nsColor&),
    NS_CONSTRUCTOR_PROPERTY(const char*, float),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsDirectionVisualizerAttribute::nsDirectionVisualizerAttribute()
  : nsVisualizerAttribute(nullptr)
{
  m_Axis = nsBasisAxis::PositiveX;
  m_fScale = 1.0f;
  m_Color = nsColor::White;
}

nsDirectionVisualizerAttribute::nsDirectionVisualizerAttribute(nsEnum<nsBasisAxis> axis, float fScale, const nsColor& fixedColor /*= nsColorScheme::LightUI(nsColorScheme::Grape)*/, const char* szColorProperty /*= nullptr*/, const char* szLengthProperty /*= nullptr*/)
  : nsVisualizerAttribute(szColorProperty, szLengthProperty)
  , m_Axis(axis)
  , m_Color(fixedColor)
  , m_fScale(fScale)
{
}

nsDirectionVisualizerAttribute::nsDirectionVisualizerAttribute(const char* szAxisProperty, float fScale, const nsColor& fixedColor /*= nsColorScheme::LightUI(nsColorScheme::Grape)*/, const char* szColorProperty /*= nullptr*/, const char* szLengthProperty /*= nullptr*/)
  : nsVisualizerAttribute(szColorProperty, szLengthProperty, szAxisProperty)
  , m_Axis(nsBasisAxis::PositiveX)
  , m_Color(fixedColor)
  , m_fScale(fScale)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsConeVisualizerAttribute, 1, nsRTTIDefaultAllocator<nsConeVisualizerAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ENUM_MEMBER_PROPERTY("Axis", nsBasisAxis, m_Axis),
    NS_MEMBER_PROPERTY("Color", m_Color),
    NS_MEMBER_PROPERTY("Scale", m_fScale),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(nsEnum<nsBasisAxis>, const char*, float, const char*, const nsColor&, const char*),
    NS_CONSTRUCTOR_PROPERTY(nsEnum<nsBasisAxis>, const char*, float, const char*, const nsColor&),
    NS_CONSTRUCTOR_PROPERTY(nsEnum<nsBasisAxis>, const char*, float, const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsConeVisualizerAttribute::nsConeVisualizerAttribute()
  : nsVisualizerAttribute(nullptr)
  , m_Axis(nsBasisAxis::PositiveX)
  , m_Color(nsColor::Red)
  , m_fScale(1.0f)
{
}

nsConeVisualizerAttribute::nsConeVisualizerAttribute(nsEnum<nsBasisAxis> axis, const char* szAngleProperty, float fScale,
  const char* szRadiusProperty, const nsColor& fixedColor /*= nsColorScheme::LightUI(nsColorScheme::Grape)*/, const char* szColorProperty)
  : nsVisualizerAttribute(szAngleProperty, szRadiusProperty, szColorProperty)
  , m_Axis(axis)
  , m_Color(fixedColor)
  , m_fScale(fScale)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCameraVisualizerAttribute, 1, nsRTTIDefaultAllocator<nsCameraVisualizerAttribute>)
{
  //NS_BEGIN_PROPERTIES
  //NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*, const char*, const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsCameraVisualizerAttribute::nsCameraVisualizerAttribute()
  : nsVisualizerAttribute(nullptr)
{
}

nsCameraVisualizerAttribute::nsCameraVisualizerAttribute(const char* szModeProperty, const char* szFovProperty, const char* szOrthoDimProperty,
  const char* szNearPlaneProperty, const char* szFarPlaneProperty)
  : nsVisualizerAttribute(szModeProperty, szFovProperty, szOrthoDimProperty, szNearPlaneProperty, szFarPlaneProperty)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMaxArraySizeAttribute, 1, nsRTTIDefaultAllocator<nsMaxArraySizeAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("MaxSize", m_uiMaxSize),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(nsUInt32),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on


//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsPreventDuplicatesAttribute, 1, nsRTTIDefaultAllocator<nsPreventDuplicatesAttribute>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsExcludeFromScript, 1, nsRTTIDefaultAllocator<nsExcludeFromScript>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsScriptableFunctionAttribute, 1, nsRTTIDefaultAllocator<nsScriptableFunctionAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ARRAY_MEMBER_PROPERTY("ArgNames", m_ArgNames),
    NS_ARRAY_MEMBER_PROPERTY("ArgTypes", m_ArgTypes),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsScriptableFunctionAttribute::nsScriptableFunctionAttribute(ArgType argType1 /*= In*/, const char* szArg1 /*= nullptr*/, ArgType argType2 /*= In*/,
  const char* szArg2 /*= nullptr*/, ArgType argType3 /*= In*/, const char* szArg3 /*= nullptr*/, ArgType argType4 /*= In*/,
  const char* szArg4 /*= nullptr*/, ArgType argType5 /*= In*/, const char* szArg5 /*= nullptr*/, ArgType argType6 /*= In*/,
  const char* szArg6 /*= nullptr*/)
{
  {
    if (nsStringUtils::IsNullOrEmpty(szArg1))
      return;

    m_ArgNames.PushBack(szArg1);
    m_ArgTypes.PushBack(argType1);
  }
  {
    if (nsStringUtils::IsNullOrEmpty(szArg2))
      return;

    m_ArgNames.PushBack(szArg2);
    m_ArgTypes.PushBack(argType2);
  }
  {
    if (nsStringUtils::IsNullOrEmpty(szArg3))
      return;

    m_ArgNames.PushBack(szArg3);
    m_ArgTypes.PushBack(argType3);
  }
  {
    if (nsStringUtils::IsNullOrEmpty(szArg4))
      return;

    m_ArgNames.PushBack(szArg4);
    m_ArgTypes.PushBack(argType4);
  }
  {
    if (nsStringUtils::IsNullOrEmpty(szArg5))
      return;

    m_ArgNames.PushBack(szArg5);
    m_ArgTypes.PushBack(argType5);
  }
  {
    if (nsStringUtils::IsNullOrEmpty(szArg6))
      return;

    m_ArgNames.PushBack(szArg6);
    m_ArgTypes.PushBack(argType6);
  }
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsFunctionArgumentAttributes, 1, nsRTTIDefaultAllocator<nsFunctionArgumentAttributes>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ArgIndex", m_uiArgIndex),
    NS_ARRAY_MEMBER_PROPERTY("ArgAttributes", m_ArgAttributes)->AddFlags(nsPropertyFlags::PointerOwner),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsFunctionArgumentAttributes::nsFunctionArgumentAttributes(nsUInt32 uiArgIndex, const nsPropertyAttribute* pAttribute1, const nsPropertyAttribute* pAttribute2 /*= nullptr*/, const nsPropertyAttribute* pAttribute3 /*= nullptr*/, const nsPropertyAttribute* pAttribute4 /*= nullptr*/)
  : m_uiArgIndex(uiArgIndex)
{
  {
    if (pAttribute1 == nullptr)
      return;

    m_ArgAttributes.PushBack(pAttribute1);
  }
  {
    if (pAttribute2 == nullptr)
      return;

    m_ArgAttributes.PushBack(pAttribute2);
  }
  {
    if (pAttribute3 == nullptr)
      return;

    m_ArgAttributes.PushBack(pAttribute3);
  }
  {
    if (pAttribute4 == nullptr)
      return;

    m_ArgAttributes.PushBack(pAttribute4);
  }
}

nsFunctionArgumentAttributes::~nsFunctionArgumentAttributes()
{
  for (auto pAttribute : m_ArgAttributes)
  {
    auto pAttributeNonConst = const_cast<nsPropertyAttribute*>(pAttribute);
    NS_DEFAULT_DELETE(pAttributeNonConst);
  }
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDynamicPinAttribute, 1, nsRTTIDefaultAllocator<nsDynamicPinAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Property", m_sProperty)
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsDynamicPinAttribute::nsDynamicPinAttribute(const char* szProperty)
  : m_sProperty(szProperty)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsLongOpAttribute, 1, nsRTTIDefaultAllocator<nsLongOpAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Type", m_sOpTypeName),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(),
    NS_CONSTRUCTOR_PROPERTY(const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsGameObjectReferenceAttribute, 1, nsRTTIDefaultAllocator<nsGameObjectReferenceAttribute>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

//////////////////////////////////////////////////////////////////////////

NS_STATICLINK_FILE(Foundation, Foundation_Reflection_Implementation_PropertyAttributes);
