#include <Foundation/FoundationPCH.h>

#include <Foundation/Reflection/Reflection.h>

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdPropertyAttribute, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdReadOnlyAttribute, 1, wdRTTIDefaultAllocator<wdReadOnlyAttribute>)
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdHiddenAttribute, 1, wdRTTIDefaultAllocator<wdHiddenAttribute>)
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdTemporaryAttribute, 1, wdRTTIDefaultAllocator<wdTemporaryAttribute>)
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_STATIC_REFLECTED_BITFLAGS(wdDependencyFlags, 1)
  WD_BITFLAGS_CONSTANTS(wdDependencyFlags::Package, wdDependencyFlags::Thumbnail, wdDependencyFlags::Transform)
WD_END_STATIC_REFLECTED_BITFLAGS;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdCategoryAttribute, 1, wdRTTIDefaultAllocator<wdCategoryAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Category", m_sCategory),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdInDevelopmentAttribute, 1, wdRTTIDefaultAllocator<wdInDevelopmentAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Phase", m_Phase),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(wdInt32),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;


const char* wdInDevelopmentAttribute::GetString() const
{
  switch (m_Phase)
  {
  case Phase::Alpha:
    return "ALPHA";

  case Phase::Beta:
    return "BETA";

    WD_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  return "";
}

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdTitleAttribute, 1, wdRTTIDefaultAllocator<wdTitleAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Title", m_sTitle),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdColorAttribute, 1, wdRTTIDefaultAllocator<wdColorAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Color", m_Color),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(wdColor),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdExposeColorAlphaAttribute, 1, wdRTTIDefaultAllocator<wdExposeColorAlphaAttribute>)
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdSuffixAttribute, 1, wdRTTIDefaultAllocator<wdSuffixAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Suffix", m_sSuffix),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdMinValueTextAttribute, 1, wdRTTIDefaultAllocator<wdMinValueTextAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Text", m_sText),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdDefaultValueAttribute, 1, wdRTTIDefaultAllocator<wdDefaultValueAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Value", m_Value),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const wdVariant&),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdClampValueAttribute, 1, wdRTTIDefaultAllocator<wdClampValueAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Min", m_MinValue),
    WD_MEMBER_PROPERTY("Max", m_MaxValue),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const wdVariant&, const wdVariant&),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdGroupAttribute, 1, wdRTTIDefaultAllocator<wdGroupAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Group", m_sGroup),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*, float),
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*, float),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

wdGroupAttribute::wdGroupAttribute()
{

}

wdGroupAttribute::wdGroupAttribute(const char* szGroup, float fOrder)
{
  m_sGroup = szGroup;
  m_fOrder = fOrder;
}

wdGroupAttribute::wdGroupAttribute(const char* szGroup, const char* szIconName, float fOrder)
{
  m_sGroup = szGroup;
  m_sIconName = szIconName;
  m_fOrder = fOrder;
}

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdTypeWidgetAttribute, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdContainerWidgetAttribute, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdTagSetWidgetAttribute, 1, wdRTTIDefaultAllocator<wdTagSetWidgetAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Filter", m_sTagFilter),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdExposedParametersAttribute, 1, wdRTTIDefaultAllocator<wdExposedParametersAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("ParametersSource", m_sParametersSource),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdDynamicDefaultValueAttribute, 1, wdRTTIDefaultAllocator<wdDynamicDefaultValueAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("ClassSource", m_sClassSource),
    WD_MEMBER_PROPERTY("ClassType", m_sClassType),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdContainerAttribute, 1, wdRTTIDefaultAllocator<wdContainerAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("CanAdd", m_bCanAdd),
    WD_MEMBER_PROPERTY("CanDelete", m_bCanDelete),
    WD_MEMBER_PROPERTY("CanMove", m_bCanMove),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(bool, bool, bool),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdConstrainPointerAttribute, 1, wdRTTIDefaultAllocator<wdConstrainPointerAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("ConstantName", m_sConstantName),
    WD_MEMBER_PROPERTY("ConstantValue", m_sConstantValueProperty),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdFileBrowserAttribute, 1, wdRTTIDefaultAllocator<wdFileBrowserAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Title", m_sDialogTitle),
    WD_MEMBER_PROPERTY("Filter", m_sTypeFilter),
    WD_MEMBER_PROPERTY("CustomAction", m_sCustomAction),
    WD_BITFLAGS_MEMBER_PROPERTY("DependencyFlags", wdDependencyFlags, m_DependencyFlags),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdAssetBrowserAttribute, 1, wdRTTIDefaultAllocator<wdAssetBrowserAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Filter", m_sTypeFilter),
    WD_BITFLAGS_MEMBER_PROPERTY("DependencyFlags", wdDependencyFlags, m_DependencyFlags),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdDynamicEnumAttribute, 1, wdRTTIDefaultAllocator<wdDynamicEnumAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
   WD_MEMBER_PROPERTY("DynamicEnum", m_sDynamicEnumName),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
   WD_CONSTRUCTOR_PROPERTY(const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdDynamicStringEnumAttribute, 1, wdRTTIDefaultAllocator<wdDynamicStringEnumAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("DynamicEnum", m_sDynamicEnumName),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdManipulatorAttribute, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Property1", m_sProperty1),
    WD_MEMBER_PROPERTY("Property2", m_sProperty2),
    WD_MEMBER_PROPERTY("Property3", m_sProperty3),
    WD_MEMBER_PROPERTY("Property4", m_sProperty4),
    WD_MEMBER_PROPERTY("Property5", m_sProperty5),
    WD_MEMBER_PROPERTY("Property6", m_sProperty6),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdManipulatorAttribute::wdManipulatorAttribute(const char* szProperty1, const char* szProperty2 /*= nullptr*/, const char* szProperty3 /*= nullptr*/,
  const char* szProperty4 /*= nullptr*/, const char* szProperty5 /*= nullptr*/, const char* szProperty6 /*= nullptr*/)
{
  m_sProperty1 = szProperty1;
  m_sProperty2 = szProperty2;
  m_sProperty3 = szProperty3;
  m_sProperty4 = szProperty4;
  m_sProperty5 = szProperty5;
  m_sProperty6 = szProperty6;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdSphereManipulatorAttribute, 1, wdRTTIDefaultAllocator<wdSphereManipulatorAttribute>)
{
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*),
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdSphereManipulatorAttribute::wdSphereManipulatorAttribute()
  : wdManipulatorAttribute(nullptr)
{
}

wdSphereManipulatorAttribute::wdSphereManipulatorAttribute(const char* szOuterRadius, const char* szInnerRadius)
  : wdManipulatorAttribute(szOuterRadius, szInnerRadius)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdCapsuleManipulatorAttribute, 1, wdRTTIDefaultAllocator<wdCapsuleManipulatorAttribute>)
{
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdCapsuleManipulatorAttribute::wdCapsuleManipulatorAttribute()
  : wdManipulatorAttribute(nullptr)
{
}

wdCapsuleManipulatorAttribute::wdCapsuleManipulatorAttribute(const char* szLength, const char* szRadius)
  : wdManipulatorAttribute(szLength, szRadius)
{
}


//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdBoxManipulatorAttribute, 1, wdRTTIDefaultAllocator<wdBoxManipulatorAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("scale", m_fSizeScale),
    WD_MEMBER_PROPERTY("recenter", m_bRecenterParent),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*, bool, float),
    WD_CONSTRUCTOR_PROPERTY(const char*, bool, float),
    WD_CONSTRUCTOR_PROPERTY(const char*, bool, float, const char*),
    WD_CONSTRUCTOR_PROPERTY(const char*, bool, float, const char*, const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdBoxManipulatorAttribute::wdBoxManipulatorAttribute()
  : wdManipulatorAttribute(nullptr)
{
}

wdBoxManipulatorAttribute::wdBoxManipulatorAttribute(const char* szSizeProperty, float fSizeScale, bool bRecenterParent, const char* szOffsetProperty, const char* szRotationProperty)
  : wdManipulatorAttribute(szSizeProperty, szOffsetProperty, szRotationProperty)
{
  m_bRecenterParent = bRecenterParent;
  m_fSizeScale = fSizeScale;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdNonUniformBoxManipulatorAttribute, 1, wdRTTIDefaultAllocator<wdNonUniformBoxManipulatorAttribute>)
{
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*),
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*, const char*, const char*, const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdNonUniformBoxManipulatorAttribute::wdNonUniformBoxManipulatorAttribute()
  : wdManipulatorAttribute(nullptr)
{
}

wdNonUniformBoxManipulatorAttribute::wdNonUniformBoxManipulatorAttribute(
  const char* szNegXProp, const char* szPosXProp, const char* szNegYProp, const char* szPosYProp, const char* szNegZProp, const char* szPosZProp)
  : wdManipulatorAttribute(szNegXProp, szPosXProp, szNegYProp, szPosYProp, szNegZProp, szPosZProp)
{
}

wdNonUniformBoxManipulatorAttribute::wdNonUniformBoxManipulatorAttribute(const char* szSizeX, const char* szSizeY, const char* szSizeZ)
  : wdManipulatorAttribute(szSizeX, szSizeY, szSizeZ)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdConeLengthManipulatorAttribute, 1, wdRTTIDefaultAllocator<wdConeLengthManipulatorAttribute>)
{
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdConeLengthManipulatorAttribute::wdConeLengthManipulatorAttribute()
  : wdManipulatorAttribute(nullptr)
{
}

wdConeLengthManipulatorAttribute::wdConeLengthManipulatorAttribute(const char* szRadiusProperty)
  : wdManipulatorAttribute(szRadiusProperty)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdConeAngleManipulatorAttribute, 1, wdRTTIDefaultAllocator<wdConeAngleManipulatorAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("scale", m_fScale),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*),
    WD_CONSTRUCTOR_PROPERTY(const char*, float),
    WD_CONSTRUCTOR_PROPERTY(const char*, float, const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdConeAngleManipulatorAttribute::wdConeAngleManipulatorAttribute()
  : wdManipulatorAttribute(nullptr)
{
  m_fScale = 1.0f;
}

wdConeAngleManipulatorAttribute::wdConeAngleManipulatorAttribute(const char* szAngleProperty, float fScale, const char* szRadiusProperty)
  : wdManipulatorAttribute(szAngleProperty, szRadiusProperty)
{
  m_fScale = fScale;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdTransformManipulatorAttribute, 1, wdRTTIDefaultAllocator<wdTransformManipulatorAttribute>)
{
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdTransformManipulatorAttribute::wdTransformManipulatorAttribute()
  : wdManipulatorAttribute(nullptr)
{
}

wdTransformManipulatorAttribute::wdTransformManipulatorAttribute(
  const char* szTranslateProperty, const char* szRotateProperty, const char* szScaleProperty, const char* szOffsetTranslation, const char* szOffsetRotation)
  : wdManipulatorAttribute(szTranslateProperty, szRotateProperty, szScaleProperty, szOffsetTranslation, szOffsetRotation)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdBoneManipulatorAttribute, 1, wdRTTIDefaultAllocator<wdBoneManipulatorAttribute>)
{
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdBoneManipulatorAttribute::wdBoneManipulatorAttribute()
  : wdManipulatorAttribute(nullptr)
{
}

wdBoneManipulatorAttribute::wdBoneManipulatorAttribute(const char* szTransformProperty, const char* szBindTo)
  : wdManipulatorAttribute(szTransformProperty, szBindTo)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_STATIC_REFLECTED_BITFLAGS(wdVisualizerAnchor, 1)
  WD_BITFLAGS_CONSTANTS(wdVisualizerAnchor::Center, wdVisualizerAnchor::PosX, wdVisualizerAnchor::NegX, wdVisualizerAnchor::PosY, wdVisualizerAnchor::NegY, wdVisualizerAnchor::PosZ, wdVisualizerAnchor::NegZ)
WD_END_STATIC_REFLECTED_BITFLAGS;
// clang-format on

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdVisualizerAttribute, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Property1", m_sProperty1),
    WD_MEMBER_PROPERTY("Property2", m_sProperty2),
    WD_MEMBER_PROPERTY("Property3", m_sProperty3),
    WD_MEMBER_PROPERTY("Property4", m_sProperty4),
    WD_MEMBER_PROPERTY("Property5", m_sProperty5),
    WD_BITFLAGS_MEMBER_PROPERTY("Anchor", wdVisualizerAnchor, m_Anchor),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdVisualizerAttribute::wdVisualizerAttribute(const char* szProperty1, const char* szProperty2 /*= nullptr*/, const char* szProperty3 /*= nullptr*/,
  const char* szProperty4 /*= nullptr*/, const char* szProperty5 /*= nullptr*/)
{
  m_sProperty1 = szProperty1;
  m_sProperty2 = szProperty2;
  m_sProperty3 = szProperty3;
  m_sProperty4 = szProperty4;
  m_sProperty5 = szProperty5;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdBoxVisualizerAttribute, 1, wdRTTIDefaultAllocator<wdBoxVisualizerAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Color", m_Color),
    WD_MEMBER_PROPERTY("OffsetOrScale", m_vOffsetOrScale),
    WD_MEMBER_PROPERTY("SizeScale", m_fSizeScale),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*, float, const wdColor&, const char*, wdBitflags<wdVisualizerAnchor>, wdVec3, const char*, const char*),
    WD_CONSTRUCTOR_PROPERTY(const char*, float, const wdColor&, const char*, wdBitflags<wdVisualizerAnchor>, wdVec3, const char*),
    WD_CONSTRUCTOR_PROPERTY(const char*, float, const wdColor&, const char*, wdBitflags<wdVisualizerAnchor>, wdVec3),
    WD_CONSTRUCTOR_PROPERTY(const char*, float, const wdColor&, const char*, wdBitflags<wdVisualizerAnchor>),
    WD_CONSTRUCTOR_PROPERTY(const char*, float, const wdColor&, const char*),
    WD_CONSTRUCTOR_PROPERTY(const char*, float, const wdColor&),
    WD_CONSTRUCTOR_PROPERTY(const char*, float),
    WD_CONSTRUCTOR_PROPERTY(const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdBoxVisualizerAttribute::wdBoxVisualizerAttribute()
  : wdVisualizerAttribute(nullptr)
{
}

wdBoxVisualizerAttribute::wdBoxVisualizerAttribute(const char* szSizeProperty, float fSizeScale, const wdColor& fixedColor /*= wdColorScheme::LightUI(wdColorScheme::Grape)*/, const char* szColorProperty /*= nullptr*/, wdBitflags<wdVisualizerAnchor> anchor /*= wdVisualizerAnchor::Center*/, wdVec3 vOffsetOrScale /*= wdVec3::ZeroVector*/, const char* szOffsetProperty /*= nullptr*/, const char* szRotationProperty /*= nullptr*/)
  : wdVisualizerAttribute(szSizeProperty, szColorProperty, szOffsetProperty, szRotationProperty)
{
  m_Color = fixedColor;
  m_vOffsetOrScale = vOffsetOrScale;
  m_Anchor = anchor;
  m_fSizeScale = fSizeScale;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdSphereVisualizerAttribute, 1, wdRTTIDefaultAllocator<wdSphereVisualizerAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Color", m_Color),
    WD_MEMBER_PROPERTY("OffsetOrScale", m_vOffsetOrScale),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*, const wdColor&, const char*, wdBitflags<wdVisualizerAnchor>, wdVec3, const char*),
    WD_CONSTRUCTOR_PROPERTY(const char*, const wdColor&, const char*, wdBitflags<wdVisualizerAnchor>, wdVec3),
    WD_CONSTRUCTOR_PROPERTY(const char*, const wdColor&, const char*, wdBitflags<wdVisualizerAnchor>),
    WD_CONSTRUCTOR_PROPERTY(const char*, const wdColor&, const char*),
    WD_CONSTRUCTOR_PROPERTY(const char*, const wdColor&),
    WD_CONSTRUCTOR_PROPERTY(const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdSphereVisualizerAttribute::wdSphereVisualizerAttribute()
  : wdVisualizerAttribute(nullptr)
{
}

wdSphereVisualizerAttribute::wdSphereVisualizerAttribute(const char* szRadiusProperty, const wdColor& fixedColor /*= wdColorScheme::LightUI(wdColorScheme::Grape)*/, const char* szColorProperty /*= nullptr*/, wdBitflags<wdVisualizerAnchor> anchor /*= wdVisualizerAnchor::Center*/, wdVec3 vOffsetOrScale /*= wdVec3::ZeroVector*/, const char* szOffsetProperty /*= nullptr*/)
  : wdVisualizerAttribute(szRadiusProperty, szColorProperty, szOffsetProperty)
{
  m_Color = fixedColor;
  m_vOffsetOrScale = vOffsetOrScale;
  m_Anchor = anchor;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdCapsuleVisualizerAttribute, 1, wdRTTIDefaultAllocator<wdCapsuleVisualizerAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Color", m_Color),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*, const wdColor&, const char*, wdBitflags<wdVisualizerAnchor>),
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*, const wdColor&, const char*),
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*, const wdColor&),
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdCapsuleVisualizerAttribute::wdCapsuleVisualizerAttribute()
  : wdVisualizerAttribute(nullptr)
{
}

wdCapsuleVisualizerAttribute::wdCapsuleVisualizerAttribute(const char* szHeightProperty, const char* szRadiusProperty, const wdColor& fixedColor /*= wdColorScheme::LightUI(wdColorScheme::Grape)*/, const char* szColorProperty /*= nullptr*/, wdBitflags<wdVisualizerAnchor> anchor /*= wdVisualizerAnchor::Center*/)
  : wdVisualizerAttribute(szHeightProperty, szRadiusProperty, szColorProperty)
{
  m_Color = fixedColor;
  m_Anchor = anchor;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdCylinderVisualizerAttribute, 1, wdRTTIDefaultAllocator<wdCylinderVisualizerAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Color", m_Color),
    WD_MEMBER_PROPERTY("OffsetOrScale", m_vOffsetOrScale),
    WD_ENUM_MEMBER_PROPERTY("Axis", wdBasisAxis, m_Axis),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(wdEnum<wdBasisAxis>, const char*, const char*, const wdColor&, const char*, wdBitflags<wdVisualizerAnchor>, wdVec3, const char*),
    WD_CONSTRUCTOR_PROPERTY(wdEnum<wdBasisAxis>, const char*, const char*, const wdColor&, const char*, wdBitflags<wdVisualizerAnchor>, wdVec3),
    WD_CONSTRUCTOR_PROPERTY(wdEnum<wdBasisAxis>, const char*, const char*, const wdColor&, const char*, wdBitflags<wdVisualizerAnchor>),
    WD_CONSTRUCTOR_PROPERTY(wdEnum<wdBasisAxis>, const char*, const char*, const wdColor&, const char*),
    WD_CONSTRUCTOR_PROPERTY(wdEnum<wdBasisAxis>, const char*, const char*, const wdColor&),
    WD_CONSTRUCTOR_PROPERTY(wdEnum<wdBasisAxis>, const char*, const char*),
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*, const wdColor&, const char*, wdBitflags<wdVisualizerAnchor>, wdVec3, const char*),
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*, const wdColor&, const char*, wdBitflags<wdVisualizerAnchor>, wdVec3),
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*, const wdColor&, const char*, wdBitflags<wdVisualizerAnchor>),
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*, const wdColor&, const char*),
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*, const wdColor&),
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdCylinderVisualizerAttribute::wdCylinderVisualizerAttribute()
  : wdVisualizerAttribute(nullptr)
{
}

wdCylinderVisualizerAttribute::wdCylinderVisualizerAttribute(wdEnum<wdBasisAxis> axis, const char* szHeightProperty, const char* szRadiusProperty, const wdColor& fixedColor /*= wdColorScheme::LightUI(wdColorScheme::Grape)*/, const char* szColorProperty /*= nullptr*/, wdBitflags<wdVisualizerAnchor> anchor /*= wdVisualizerAnchor::Center*/, wdVec3 vOffsetOrScale /*= wdVec3::ZeroVector*/, const char* szOffsetProperty /*= nullptr*/)
  : wdVisualizerAttribute(szHeightProperty, szRadiusProperty, szColorProperty, szOffsetProperty)
{
  m_Color = fixedColor;
  m_vOffsetOrScale = vOffsetOrScale;
  m_Axis = axis;
  m_Anchor = anchor;
}

wdCylinderVisualizerAttribute::wdCylinderVisualizerAttribute(const char* szAxisProperty, const char* szHeightProperty, const char* szRadiusProperty, const wdColor& fixedColor /*= wdColorScheme::LightUI(wdColorScheme::Grape)*/, const char* szColorProperty /*= nullptr*/, wdBitflags<wdVisualizerAnchor> anchor /*= wdVisualizerAnchor::Center*/, wdVec3 vOffsetOrScale /*= wdVec3::ZeroVector()*/, const char* szOffsetProperty /*= nullptr*/)
  : wdVisualizerAttribute(szHeightProperty, szRadiusProperty, szColorProperty, szOffsetProperty, szAxisProperty)
{
  m_Color = fixedColor;
  m_vOffsetOrScale = vOffsetOrScale;
  m_Axis = wdBasisAxis::Default;
  m_Anchor = anchor;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdDirectionVisualizerAttribute, 1, wdRTTIDefaultAllocator<wdDirectionVisualizerAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_ENUM_MEMBER_PROPERTY("Axis", wdBasisAxis, m_Axis),
    WD_MEMBER_PROPERTY("Color", m_Color),
    WD_MEMBER_PROPERTY("Scale", m_fScale)
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(wdEnum<wdBasisAxis>, float, const wdColor&, const char*, const char*),
    WD_CONSTRUCTOR_PROPERTY(wdEnum<wdBasisAxis>, float, const wdColor&, const char*),
    WD_CONSTRUCTOR_PROPERTY(wdEnum<wdBasisAxis>, float, const wdColor&),
    WD_CONSTRUCTOR_PROPERTY(wdEnum<wdBasisAxis>, float),
    WD_CONSTRUCTOR_PROPERTY(const char*, float, const wdColor&, const char*, const char*),
    WD_CONSTRUCTOR_PROPERTY(const char*, float, const wdColor&, const char*),
    WD_CONSTRUCTOR_PROPERTY(const char*, float, const wdColor&),
    WD_CONSTRUCTOR_PROPERTY(const char*, float),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdDirectionVisualizerAttribute::wdDirectionVisualizerAttribute()
  : wdVisualizerAttribute(nullptr)
{
  m_Axis = wdBasisAxis::PositiveX;
  m_fScale = 1.0f;
  m_Color = wdColor::White;
}

wdDirectionVisualizerAttribute::wdDirectionVisualizerAttribute(wdEnum<wdBasisAxis> axis, float fScale, const wdColor& fixedColor /*= wdColorScheme::LightUI(wdColorScheme::Grape)*/, const char* szColorProperty /*= nullptr*/, const char* szLengthProperty /*= nullptr*/)
  : wdVisualizerAttribute(szColorProperty, szLengthProperty)
{
  m_Axis = axis;
  m_fScale = fScale;
  m_Color = fixedColor;
}

wdDirectionVisualizerAttribute::wdDirectionVisualizerAttribute(const char* szAxisProperty, float fScale, const wdColor& fixedColor /*= wdColorScheme::LightUI(wdColorScheme::Grape)*/, const char* szColorProperty /*= nullptr*/, const char* szLengthProperty /*= nullptr*/)
  : wdVisualizerAttribute(szColorProperty, szLengthProperty, szAxisProperty)
{
  m_Axis = wdBasisAxis::PositiveX;
  m_fScale = fScale;
  m_Color = fixedColor;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdConeVisualizerAttribute, 1, wdRTTIDefaultAllocator<wdConeVisualizerAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_ENUM_MEMBER_PROPERTY("Axis", wdBasisAxis, m_Axis),
    WD_MEMBER_PROPERTY("Color", m_Color),
    WD_MEMBER_PROPERTY("Scale", m_fScale),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(wdEnum<wdBasisAxis>, const char*, float, const char*, const wdColor&, const char*),
    WD_CONSTRUCTOR_PROPERTY(wdEnum<wdBasisAxis>, const char*, float, const char*, const wdColor&),
    WD_CONSTRUCTOR_PROPERTY(wdEnum<wdBasisAxis>, const char*, float, const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdConeVisualizerAttribute::wdConeVisualizerAttribute()
  : wdVisualizerAttribute(nullptr)
{
  m_Axis = wdBasisAxis::PositiveX;
  m_Color = wdColor::Red;
  m_fScale = 1.0f;
}

wdConeVisualizerAttribute::wdConeVisualizerAttribute(wdEnum<wdBasisAxis> axis, const char* szAngleProperty, float fScale,
  const char* szRadiusProperty, const wdColor& fixedColor /*= wdColorScheme::LightUI(wdColorScheme::Grape)*/, const char* szColorProperty)
  : wdVisualizerAttribute(szAngleProperty, szRadiusProperty, szColorProperty)
{
  m_Axis = axis;
  m_Color = fixedColor;
  m_fScale = fScale;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdCameraVisualizerAttribute, 1, wdRTTIDefaultAllocator<wdCameraVisualizerAttribute>)
{
  //WD_BEGIN_PROPERTIES
  //WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*, const char*, const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdCameraVisualizerAttribute::wdCameraVisualizerAttribute()
  : wdVisualizerAttribute(nullptr)
{
}

wdCameraVisualizerAttribute::wdCameraVisualizerAttribute(const char* szModeProperty, const char* szFovProperty, const char* szOrthoDimProperty,
  const char* szNearPlaneProperty, const char* szFarPlaneProperty)
  : wdVisualizerAttribute(szModeProperty, szFovProperty, szOrthoDimProperty, szNearPlaneProperty, szFarPlaneProperty)
{
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdMaxArraySizeAttribute, 1, wdRTTIDefaultAllocator<wdMaxArraySizeAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("MaxSize", m_uiMaxSize),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(wdUInt32),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on


//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdPreventDuplicatesAttribute, 1, wdRTTIDefaultAllocator<wdPreventDuplicatesAttribute>)
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdAutoGenVisScriptMsgSender, 1, wdRTTIDefaultAllocator<wdAutoGenVisScriptMsgSender>)
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdAutoGenVisScriptMsgHandler, 1, wdRTTIDefaultAllocator<wdAutoGenVisScriptMsgHandler>)
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdScriptableFunctionAttribute, 1, wdRTTIDefaultAllocator<wdScriptableFunctionAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Arg1", m_sArg1),
    WD_MEMBER_PROPERTY("Arg2", m_sArg2),
    WD_MEMBER_PROPERTY("Arg3", m_sArg3),
    WD_MEMBER_PROPERTY("Arg4", m_sArg4),
    WD_MEMBER_PROPERTY("Arg5", m_sArg5),
    WD_MEMBER_PROPERTY("Arg6", m_sArg6),
    WD_MEMBER_PROPERTY("ArgType1", m_ArgType1),
    WD_MEMBER_PROPERTY("ArgType2", m_ArgType2),
    WD_MEMBER_PROPERTY("ArgType3", m_ArgType3),
    WD_MEMBER_PROPERTY("ArgType4", m_ArgType4),
    WD_MEMBER_PROPERTY("ArgType5", m_ArgType5),
    WD_MEMBER_PROPERTY("ArgType6", m_ArgType6),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdScriptableFunctionAttribute::wdScriptableFunctionAttribute(ArgType argType1 /*= In*/, const char* szArg1 /*= nullptr*/, ArgType argType2 /*= In*/,
  const char* szArg2 /*= nullptr*/, ArgType argType3 /*= In*/, const char* szArg3 /*= nullptr*/, ArgType argType4 /*= In*/,
  const char* szArg4 /*= nullptr*/, ArgType argType5 /*= In*/, const char* szArg5 /*= nullptr*/, ArgType argType6 /*= In*/,
  const char* szArg6 /*= nullptr*/)
{
  m_sArg1 = szArg1;
  m_sArg2 = szArg2;
  m_sArg3 = szArg3;
  m_sArg4 = szArg4;
  m_sArg5 = szArg5;
  m_sArg6 = szArg6;

  m_ArgType1 = argType1;
  m_ArgType2 = argType2;
  m_ArgType3 = argType3;
  m_ArgType4 = argType4;
  m_ArgType5 = argType5;
  m_ArgType6 = argType6;
}

const char* wdScriptableFunctionAttribute::GetArgumentName(wdUInt32 uiIndex) const
{
  switch (uiIndex)
  {
    case 0:
      return m_sArg1;
    case 1:
      return m_sArg2;
    case 2:
      return m_sArg3;
    case 3:
      return m_sArg4;
    case 4:
      return m_sArg5;
    case 5:
      return m_sArg6;
  }

  WD_ASSERT_NOT_IMPLEMENTED;
  return nullptr;
}

wdScriptableFunctionAttribute::ArgType wdScriptableFunctionAttribute::GetArgumentType(wdUInt32 uiIndex) const
{
  switch (uiIndex)
  {
    case 0:
      return (ArgType)m_ArgType1;
    case 1:
      return (ArgType)m_ArgType2;
    case 2:
      return (ArgType)m_ArgType3;
    case 3:
      return (ArgType)m_ArgType4;
    case 4:
      return (ArgType)m_ArgType5;
    case 5:
      return (ArgType)m_ArgType6;
  }

  WD_ASSERT_NOT_IMPLEMENTED;
  return ArgType::In;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdVisScriptMappingAttribute, 1, wdRTTIDefaultAllocator<wdVisScriptMappingAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Mapping", m_iMapping)
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdLongOpAttribute, 1, wdRTTIDefaultAllocator<wdLongOpAttribute>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Type", m_sOpTypeName),
  }
  WD_END_PROPERTIES;
  WD_BEGIN_FUNCTIONS
  {
    WD_CONSTRUCTOR_PROPERTY(),
    WD_CONSTRUCTOR_PROPERTY(const char*),
  }
  WD_END_FUNCTIONS;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdGameObjectReferenceAttribute, 1, wdRTTIDefaultAllocator<wdGameObjectReferenceAttribute>)
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

//////////////////////////////////////////////////////////////////////////

WD_STATICLINK_FILE(Foundation, Foundation_Reflection_Implementation_PropertyAttributes);
