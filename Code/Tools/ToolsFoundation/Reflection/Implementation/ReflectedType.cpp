#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>
#include <ToolsFoundation/Reflection/PhantomRttiManager.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>

// clang-format off
WD_BEGIN_STATIC_REFLECTED_TYPE(wdAttributeHolder, wdNoBase, 1, wdRTTINoAllocator)
{
  flags.Add(wdTypeFlags::Abstract);
  WD_BEGIN_PROPERTIES
  {
    WD_ARRAY_ACCESSOR_PROPERTY("Attributes", GetCount, GetValue, SetValue, Insert, Remove)->AddFlags(wdPropertyFlags::PointerOwner),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;
// clang-format on

wdAttributeHolder::wdAttributeHolder() = default;

wdAttributeHolder::wdAttributeHolder(const wdAttributeHolder& rhs)
{
  m_Attributes = rhs.m_Attributes;
  rhs.m_Attributes.Clear();

  m_ReferenceAttributes = rhs.m_ReferenceAttributes;
}

wdAttributeHolder::~wdAttributeHolder()
{
  for (auto pAttr : m_Attributes)
  {
    pAttr->GetDynamicRTTI()->GetAllocator()->Deallocate(pAttr);
  }
}

void wdAttributeHolder::operator=(const wdAttributeHolder& rhs)
{
  if (this == &rhs)
    return;

  m_Attributes = rhs.m_Attributes;
  rhs.m_Attributes.Clear();

  m_ReferenceAttributes = rhs.m_ReferenceAttributes;
}

wdUInt32 wdAttributeHolder::GetCount() const
{
  return wdMath::Max(m_ReferenceAttributes.GetCount(), m_Attributes.GetCount());
}

wdPropertyAttribute* wdAttributeHolder::GetValue(wdUInt32 uiIndex) const
{
  if (!m_ReferenceAttributes.IsEmpty())
    return m_ReferenceAttributes[uiIndex];

  return m_Attributes[uiIndex];
}

void wdAttributeHolder::SetValue(wdUInt32 uiIndex, wdPropertyAttribute* value)
{
  m_Attributes[uiIndex] = value;
}

void wdAttributeHolder::Insert(wdUInt32 uiIndex, wdPropertyAttribute* value)
{
  m_Attributes.Insert(value, uiIndex);
}

void wdAttributeHolder::Remove(wdUInt32 uiIndex)
{
  m_Attributes.RemoveAtAndCopy(uiIndex);
}

////////////////////////////////////////////////////////////////////////
// wdReflectedPropertyDescriptor
////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_STATIC_REFLECTED_TYPE(wdReflectedPropertyDescriptor, wdAttributeHolder, 2, wdRTTIDefaultAllocator<wdReflectedPropertyDescriptor>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_ENUM_MEMBER_PROPERTY("Category", wdPropertyCategory, m_Category),
    WD_MEMBER_PROPERTY("Name", m_sName),
    WD_MEMBER_PROPERTY("Type", m_sType),
    WD_BITFLAGS_MEMBER_PROPERTY("Flags", wdPropertyFlags, m_Flags),
    WD_MEMBER_PROPERTY("ConstantValue", m_ConstantValue),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;
// clang-format on

class wdReflectedPropertyDescriptorPatch_1_2 : public wdGraphPatch
{
public:
  wdReflectedPropertyDescriptorPatch_1_2()
    : wdGraphPatch("wdReflectedPropertyDescriptor", 2)
  {
  }

  virtual void Patch(wdGraphPatchContext& ref_context, wdAbstractObjectGraph* pGraph, wdAbstractObjectNode* pNode) const override
  {
    if (wdAbstractObjectNode::Property* pProp = pNode->FindProperty("Flags"))
    {
      wdStringBuilder sValue = pProp->m_Value.Get<wdString>();
      wdHybridArray<wdStringView, 32> values;
      sValue.Split(false, values, "|");

      wdStringBuilder sNewValue;
      for (wdInt32 i = (wdInt32)values.GetCount() - 1; i >= 0; i--)
      {
        if (values[i].IsEqual("wdPropertyFlags::Constant"))
        {
          values.RemoveAtAndCopy(i);
        }
        else if (values[i].IsEqual("wdPropertyFlags::EmbeddedClass"))
        {
          values[i] = wdStringView("wdPropertyFlags::Class");
        }
        else if (values[i].IsEqual("wdPropertyFlags::Pointer"))
        {
          values.PushBack(wdStringView("wdPropertyFlags::Class"));
        }
      }
      for (wdUInt32 i = 0; i < values.GetCount(); ++i)
      {
        if (i != 0)
          sNewValue.Append("|");
        sNewValue.Append(values[i]);
      }
      pProp->m_Value = sNewValue.GetData();
    }
  }
};

wdReflectedPropertyDescriptorPatch_1_2 g_wdReflectedPropertyDescriptorPatch_1_2;


wdReflectedPropertyDescriptor::wdReflectedPropertyDescriptor(
  wdPropertyCategory::Enum category, const char* szName, const char* szType, wdBitflags<wdPropertyFlags> flags)
  : m_Category(category)
  , m_sName(szName)
  , m_sType(szType)
  , m_Flags(flags)
{
}

wdReflectedPropertyDescriptor::wdReflectedPropertyDescriptor(wdPropertyCategory::Enum category, const char* szName, const char* szType,
  wdBitflags<wdPropertyFlags> flags, const wdArrayPtr<wdPropertyAttribute* const> attributes)
  : m_Category(category)
  , m_sName(szName)
  , m_sType(szType)
  , m_Flags(flags)
{
  m_ReferenceAttributes = attributes;
}

wdReflectedPropertyDescriptor::wdReflectedPropertyDescriptor(
  const char* szName, const wdVariant& constantValue, const wdArrayPtr<wdPropertyAttribute* const> attributes)
  : m_Category(wdPropertyCategory::Constant)
  , m_sName(szName)
  , m_sType()
  , m_Flags(wdPropertyFlags::StandardType | wdPropertyFlags::ReadOnly)
  , m_ConstantValue(constantValue)
{
  m_ReferenceAttributes = attributes;
  const wdRTTI* pType = wdReflectionUtils::GetTypeFromVariant(constantValue);
  if (pType)
    m_sType = pType->GetTypeName();
}

wdReflectedPropertyDescriptor::wdReflectedPropertyDescriptor(const wdReflectedPropertyDescriptor& rhs)
{
  operator=(rhs);
}

void wdReflectedPropertyDescriptor::operator=(const wdReflectedPropertyDescriptor& rhs)
{
  m_Category = rhs.m_Category;
  m_sName = rhs.m_sName;

  m_sType = rhs.m_sType;

  m_Flags = rhs.m_Flags;
  m_ConstantValue = rhs.m_ConstantValue;

  wdAttributeHolder::operator=(rhs);
}

wdReflectedPropertyDescriptor::~wdReflectedPropertyDescriptor() {}


////////////////////////////////////////////////////////////////////////
// wdFunctionParameterDescriptor
////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_STATIC_REFLECTED_TYPE(wdFunctionArgumentDescriptor, wdNoBase, 1, wdRTTIDefaultAllocator<wdFunctionArgumentDescriptor>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Type", m_sType),
    WD_BITFLAGS_MEMBER_PROPERTY("Flags", wdPropertyFlags, m_Flags),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;
// clang-format on

wdFunctionArgumentDescriptor::wdFunctionArgumentDescriptor() {}

wdFunctionArgumentDescriptor::wdFunctionArgumentDescriptor(const char* szType, wdBitflags<wdPropertyFlags> flags)
  : m_sType(szType)
  , m_Flags(flags)
{
}


////////////////////////////////////////////////////////////////////////
// wdReflectedFunctionDescriptor
////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_STATIC_REFLECTED_TYPE(wdReflectedFunctionDescriptor, wdAttributeHolder, 1, wdRTTIDefaultAllocator<wdReflectedFunctionDescriptor>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Name", m_sName),
    WD_BITFLAGS_MEMBER_PROPERTY("Flags", wdPropertyFlags, m_Flags),
    WD_ENUM_MEMBER_PROPERTY("Type", wdFunctionType, m_Type),
    WD_MEMBER_PROPERTY("ReturnValue", m_ReturnValue),
    WD_ARRAY_MEMBER_PROPERTY("Arguments", m_Arguments),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;
// clang-format on

wdReflectedFunctionDescriptor::wdReflectedFunctionDescriptor() {}

wdReflectedFunctionDescriptor::wdReflectedFunctionDescriptor(
  const char* szName, wdBitflags<wdPropertyFlags> flags, wdEnum<wdFunctionType> type, const wdArrayPtr<wdPropertyAttribute* const> attributes)
  : m_sName(szName)
  , m_Flags(flags)
  , m_Type(type)
{
  m_ReferenceAttributes = attributes;
}

wdReflectedFunctionDescriptor::wdReflectedFunctionDescriptor(const wdReflectedFunctionDescriptor& rhs)
{
  operator=(rhs);
}

wdReflectedFunctionDescriptor::~wdReflectedFunctionDescriptor() {}

void wdReflectedFunctionDescriptor::operator=(const wdReflectedFunctionDescriptor& rhs)
{
  m_sName = rhs.m_sName;
  m_Flags = rhs.m_Flags;
  m_ReturnValue = rhs.m_ReturnValue;
  m_Arguments = rhs.m_Arguments;
  wdAttributeHolder::operator=(rhs);
}

////////////////////////////////////////////////////////////////////////
// wdReflectedTypeDescriptor
////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_STATIC_REFLECTED_TYPE(wdReflectedTypeDescriptor, wdAttributeHolder, 1, wdRTTIDefaultAllocator<wdReflectedTypeDescriptor>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("TypeName", m_sTypeName),
    WD_MEMBER_PROPERTY("PluginName", m_sPluginName),
    WD_MEMBER_PROPERTY("ParentTypeName", m_sParentTypeName),
    WD_BITFLAGS_MEMBER_PROPERTY("Flags", wdTypeFlags, m_Flags),
    WD_ARRAY_MEMBER_PROPERTY("Properties", m_Properties),
    WD_ARRAY_MEMBER_PROPERTY("Functions", m_Functions),
    WD_MEMBER_PROPERTY("TypeVersion", m_uiTypeVersion),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;
// clang-format on

wdReflectedTypeDescriptor::~wdReflectedTypeDescriptor() {}
