#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>
#include <ToolsFoundation/Reflection/PhantomRttiManager.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsAttributeHolder, nsNoBase, 1, nsRTTINoAllocator)
{
  flags.Add(nsTypeFlags::Abstract);
  NS_BEGIN_PROPERTIES
  {
    NS_ARRAY_ACCESSOR_PROPERTY("Attributes", GetCount, GetValue, SetValue, Insert, Remove)->AddFlags(nsPropertyFlags::PointerOwner),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

nsAttributeHolder::nsAttributeHolder() = default;

nsAttributeHolder::nsAttributeHolder(const nsAttributeHolder& rhs)
{
  m_Attributes = rhs.m_Attributes;
  rhs.m_Attributes.Clear();

  m_ReferenceAttributes = rhs.m_ReferenceAttributes;
}

nsAttributeHolder::~nsAttributeHolder()
{
  for (auto pAttr : m_Attributes)
  {
    pAttr->GetDynamicRTTI()->GetAllocator()->Deallocate(const_cast<nsPropertyAttribute*>(pAttr));
  }
}

void nsAttributeHolder::operator=(const nsAttributeHolder& rhs)
{
  if (this == &rhs)
    return;

  m_Attributes = rhs.m_Attributes;
  rhs.m_Attributes.Clear();

  m_ReferenceAttributes = rhs.m_ReferenceAttributes;
}

nsUInt32 nsAttributeHolder::GetCount() const
{
  return nsMath::Max(m_ReferenceAttributes.GetCount(), m_Attributes.GetCount());
}

const nsPropertyAttribute* nsAttributeHolder::GetValue(nsUInt32 uiIndex) const
{
  if (!m_ReferenceAttributes.IsEmpty())
    return m_ReferenceAttributes[uiIndex];

  return m_Attributes[uiIndex];
}

void nsAttributeHolder::SetValue(nsUInt32 uiIndex, const nsPropertyAttribute* value)
{
  m_Attributes[uiIndex] = value;
}

void nsAttributeHolder::Insert(nsUInt32 uiIndex, const nsPropertyAttribute* value)
{
  m_Attributes.InsertAt(uiIndex, value);
}

void nsAttributeHolder::Remove(nsUInt32 uiIndex)
{
  m_Attributes.RemoveAtAndCopy(uiIndex);
}

////////////////////////////////////////////////////////////////////////
// nsReflectedPropertyDescriptor
////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsReflectedPropertyDescriptor, nsAttributeHolder, 2, nsRTTIDefaultAllocator<nsReflectedPropertyDescriptor>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ENUM_MEMBER_PROPERTY("Category", nsPropertyCategory, m_Category),
    NS_MEMBER_PROPERTY("Name", m_sName),
    NS_MEMBER_PROPERTY("Type", m_sType),
    NS_BITFLAGS_MEMBER_PROPERTY("Flags", nsPropertyFlags, m_Flags),
    NS_MEMBER_PROPERTY("ConstantValue", m_ConstantValue),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

class nsReflectedPropertyDescriptorPatch_1_2 : public nsGraphPatch
{
public:
  nsReflectedPropertyDescriptorPatch_1_2()
    : nsGraphPatch("nsReflectedPropertyDescriptor", 2)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    if (nsAbstractObjectNode::Property* pProp = pNode->FindProperty("Flags"))
    {
      nsStringBuilder sValue = pProp->m_Value.Get<nsString>();
      nsHybridArray<nsStringView, 32> values;
      sValue.Split(false, values, "|");

      nsStringBuilder sNewValue;
      for (nsInt32 i = (nsInt32)values.GetCount() - 1; i >= 0; i--)
      {
        if (values[i].IsEqual("nsPropertyFlags::Constant"))
        {
          values.RemoveAtAndCopy(i);
        }
        else if (values[i].IsEqual("nsPropertyFlags::EmbeddedClass"))
        {
          values[i] = nsStringView("nsPropertyFlags::Class");
        }
        else if (values[i].IsEqual("nsPropertyFlags::Pointer"))
        {
          values.PushBack(nsStringView("nsPropertyFlags::Class"));
        }
      }
      for (nsUInt32 i = 0; i < values.GetCount(); ++i)
      {
        if (i != 0)
          sNewValue.Append("|");
        sNewValue.Append(values[i]);
      }
      pProp->m_Value = sNewValue.GetData();
    }
  }
};

nsReflectedPropertyDescriptorPatch_1_2 g_nsReflectedPropertyDescriptorPatch_1_2;


nsReflectedPropertyDescriptor::nsReflectedPropertyDescriptor(nsPropertyCategory::Enum category, nsStringView sName, nsStringView sType, nsBitflags<nsPropertyFlags> flags)
  : m_Category(category)
  , m_sName(sName)
  , m_sType(sType)
  , m_Flags(flags)
{
}

nsReflectedPropertyDescriptor::nsReflectedPropertyDescriptor(nsPropertyCategory::Enum category, nsStringView sName, nsStringView sType,
  nsBitflags<nsPropertyFlags> flags, nsArrayPtr<const nsPropertyAttribute* const> attributes)
  : m_Category(category)
  , m_sName(sName)
  , m_sType(sType)
  , m_Flags(flags)
{
  m_ReferenceAttributes = attributes;
}

nsReflectedPropertyDescriptor::nsReflectedPropertyDescriptor(
  nsStringView sName, const nsVariant& constantValue, nsArrayPtr<const nsPropertyAttribute* const> attributes)
  : m_Category(nsPropertyCategory::Constant)
  , m_sName(sName)
  , m_sType()
  , m_Flags(nsPropertyFlags::StandardType | nsPropertyFlags::ReadOnly)
  , m_ConstantValue(constantValue)
{
  m_ReferenceAttributes = attributes;
  const nsRTTI* pType = nsReflectionUtils::GetTypeFromVariant(constantValue);
  if (pType)
    m_sType = pType->GetTypeName();
}

nsReflectedPropertyDescriptor::nsReflectedPropertyDescriptor(const nsReflectedPropertyDescriptor& rhs)
{
  operator=(rhs);
}

void nsReflectedPropertyDescriptor::operator=(const nsReflectedPropertyDescriptor& rhs)
{
  m_Category = rhs.m_Category;
  m_sName = rhs.m_sName;

  m_sType = rhs.m_sType;

  m_Flags = rhs.m_Flags;
  m_ConstantValue = rhs.m_ConstantValue;

  nsAttributeHolder::operator=(rhs);
}

nsReflectedPropertyDescriptor::~nsReflectedPropertyDescriptor() = default;


////////////////////////////////////////////////////////////////////////
// nsFunctionParameterDescriptor
////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsFunctionArgumentDescriptor, nsNoBase, 1, nsRTTIDefaultAllocator<nsFunctionArgumentDescriptor>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Type", m_sType),
    NS_BITFLAGS_MEMBER_PROPERTY("Flags", nsPropertyFlags, m_Flags),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

nsFunctionArgumentDescriptor::nsFunctionArgumentDescriptor() = default;

nsFunctionArgumentDescriptor::nsFunctionArgumentDescriptor(nsStringView sType, nsBitflags<nsPropertyFlags> flags)
  : m_sType(sType)
  , m_Flags(flags)
{
}


////////////////////////////////////////////////////////////////////////
// nsReflectedFunctionDescriptor
////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsReflectedFunctionDescriptor, nsAttributeHolder, 1, nsRTTIDefaultAllocator<nsReflectedFunctionDescriptor>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Name", m_sName),
    NS_BITFLAGS_MEMBER_PROPERTY("Flags", nsPropertyFlags, m_Flags),
    NS_ENUM_MEMBER_PROPERTY("Type", nsFunctionType, m_Type),
    NS_MEMBER_PROPERTY("ReturnValue", m_ReturnValue),
    NS_ARRAY_MEMBER_PROPERTY("Arguments", m_Arguments),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

nsReflectedFunctionDescriptor::nsReflectedFunctionDescriptor() = default;

nsReflectedFunctionDescriptor::nsReflectedFunctionDescriptor(nsStringView sName, nsBitflags<nsPropertyFlags> flags, nsEnum<nsFunctionType> type, nsArrayPtr<const nsPropertyAttribute* const> attributes)
  : m_sName(sName)
  , m_Flags(flags)
  , m_Type(type)
{
  m_ReferenceAttributes = attributes;
}

nsReflectedFunctionDescriptor::nsReflectedFunctionDescriptor(const nsReflectedFunctionDescriptor& rhs)
{
  operator=(rhs);
}

nsReflectedFunctionDescriptor::~nsReflectedFunctionDescriptor() = default;

void nsReflectedFunctionDescriptor::operator=(const nsReflectedFunctionDescriptor& rhs)
{
  m_sName = rhs.m_sName;
  m_Flags = rhs.m_Flags;
  m_Type = rhs.m_Type;
  m_ReturnValue = rhs.m_ReturnValue;
  m_Arguments = rhs.m_Arguments;
  nsAttributeHolder::operator=(rhs);
}

////////////////////////////////////////////////////////////////////////
// nsReflectedTypeDescriptor
////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsReflectedTypeDescriptor, nsAttributeHolder, 1, nsRTTIDefaultAllocator<nsReflectedTypeDescriptor>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("TypeName", m_sTypeName),
    NS_MEMBER_PROPERTY("PluginName", m_sPluginName),
    NS_MEMBER_PROPERTY("ParentTypeName", m_sParentTypeName),
    NS_BITFLAGS_MEMBER_PROPERTY("Flags", nsTypeFlags, m_Flags),
    NS_ARRAY_MEMBER_PROPERTY("Properties", m_Properties),
    NS_ARRAY_MEMBER_PROPERTY("Functions", m_Functions),
    NS_MEMBER_PROPERTY("TypeVersion", m_uiTypeVersion),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

nsReflectedTypeDescriptor::~nsReflectedTypeDescriptor() = default;
