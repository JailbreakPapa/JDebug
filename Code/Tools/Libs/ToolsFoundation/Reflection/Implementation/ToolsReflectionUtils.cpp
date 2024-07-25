#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/ReflectionSerializer.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <Foundation/Types/VariantTypeRegistry.h>
#include <ToolsFoundation/Object/DocumentObjectBase.h>
#include <ToolsFoundation/Reflection/IReflectedTypeAccessor.h>
#include <ToolsFoundation/Reflection/PhantomRttiManager.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>
#include <ToolsFoundation/Reflection/ToolsReflectionUtils.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

namespace
{
  struct GetDoubleFunc
  {
    GetDoubleFunc(const nsVariant& value)
      : m_Value(value)
    {
    }
    template <typename T>
    void operator()()
    {
      if (m_Value.CanConvertTo<double>())
      {
        m_fValue = m_Value.ConvertTo<double>();
        m_bValid = true;
      }
    }

    const nsVariant& m_Value;
    double m_fValue = 0;
    bool m_bValid = false;
  };

  template <>
  void GetDoubleFunc::operator()<nsAngle>()
  {
    m_fValue = m_Value.Get<nsAngle>().GetDegree();
    m_bValid = true;
  }

  template <>
  void GetDoubleFunc::operator()<nsTime>()
  {
    m_fValue = m_Value.Get<nsTime>().GetSeconds();
    m_bValid = true;
  }

  struct GetVariantFunc
  {
    GetVariantFunc(double fValue, nsVariantType::Enum type, nsVariant& out_value)
      : m_fValue(fValue)
      , m_Type(type)
      , m_Value(out_value)
    {
    }
    template <typename T>
    void operator()()
    {
      m_Value = m_fValue;
      if (m_Value.CanConvertTo(m_Type))
      {
        m_Value = m_Value.ConvertTo(m_Type);
        m_bValid = true;
      }
      else
      {
        m_Value = nsVariant();
      }
    }

    double m_fValue;
    nsVariantType::Enum m_Type;
    nsVariant& m_Value;
    bool m_bValid = false;
  };

  template <>
  void GetVariantFunc::operator()<nsAngle>()
  {
    m_Value = nsAngle::MakeFromDegree((float)m_fValue);
    m_bValid = true;
  }

  template <>
  void GetVariantFunc::operator()<nsTime>()
  {
    m_Value = nsTime::MakeFromSeconds(m_fValue);
    m_bValid = true;
  }
} // namespace
////////////////////////////////////////////////////////////////////////
// nsToolsReflectionUtils public functions
////////////////////////////////////////////////////////////////////////

nsVariantType::Enum nsToolsReflectionUtils::GetStorageType(const nsAbstractProperty* pProperty)
{
  nsVariantType::Enum type = nsVariantType::Uuid;

  const bool bIsValueType = nsReflectionUtils::IsValueType(pProperty);

  switch (pProperty->GetCategory())
  {
    case nsPropertyCategory::Member:
    {
      if (bIsValueType)
        type = pProperty->GetSpecificType()->GetVariantType();
      else if (pProperty->GetFlags().IsAnySet(nsPropertyFlags::IsEnum | nsPropertyFlags::Bitflags))
        type = nsVariantType::Int64;
    }
    break;
    case nsPropertyCategory::Array:
    case nsPropertyCategory::Set:
    {
      type = nsVariantType::VariantArray;
    }
    break;
    case nsPropertyCategory::Map:
    {
      type = nsVariantType::VariantDictionary;
    }
    break;
    default:
      break;
  }

  // We can't 'store' a string view as it has no ownership of its own. Thus, all string views are stored as strings instead.
  if (type == nsVariantType::StringView)
    type = nsVariantType::String;

  return type;
}

nsVariant nsToolsReflectionUtils::GetStorageDefault(const nsAbstractProperty* pProperty)
{
  const nsDefaultValueAttribute* pAttrib = pProperty->GetAttributeByType<nsDefaultValueAttribute>();
  const bool bIsValueType = nsReflectionUtils::IsValueType(pProperty);

  switch (pProperty->GetCategory())
  {
    case nsPropertyCategory::Member:
    {
      const nsVariantType::Enum memberType = GetStorageType(pProperty);
      nsVariant value = nsReflectionUtils::GetDefaultValue(pProperty);
      // Sometimes, the default value does not match the storage type, e.g. nsStringView is stored as nsString as it needs to be stored in the editor representation, but the reflection can still return default values matching nsStringView (constants for example).
      if (bIsValueType && value.GetType() != memberType)
        value = value.ConvertTo(memberType);

      NS_ASSERT_DEBUG(!value.IsValid() || memberType == value.GetType(), "Default value type does not match the storage type of the property");
      return value;
    }
    break;
    case nsPropertyCategory::Array:
    case nsPropertyCategory::Set:
    {
      if (bIsValueType && pAttrib && pAttrib->GetValue().IsA<nsVariantArray>())
      {
        auto elementType = pProperty->GetFlags().IsSet(nsPropertyFlags::StandardType) ? pProperty->GetSpecificType()->GetVariantType() : nsVariantType::Uuid;

        const nsVariantArray& value = pAttrib->GetValue().Get<nsVariantArray>();
        nsVariantArray ret;
        ret.SetCount(value.GetCount());
        for (nsUInt32 i = 0; i < value.GetCount(); i++)
        {
          ret[i] = value[i].ConvertTo(elementType);
        }
        return ret;
      }
      return nsVariantArray();
    }
    break;
    case nsPropertyCategory::Map:
    {
      return nsVariantDictionary();
    }
    break;
    case nsPropertyCategory::Constant:
    case nsPropertyCategory::Function:
      break; // no defaults
  }
  return nsVariant();
}

bool nsToolsReflectionUtils::GetFloatFromVariant(const nsVariant& val, double& out_fValue)
{
  if (val.IsValid())
  {
    GetDoubleFunc func(val);
    nsVariant::DispatchTo(func, val.GetType());
    out_fValue = func.m_fValue;
    return func.m_bValid;
  }
  return false;
}


bool nsToolsReflectionUtils::GetVariantFromFloat(double fValue, nsVariantType::Enum type, nsVariant& out_val)
{
  GetVariantFunc func(fValue, type, out_val);
  nsVariant::DispatchTo(func, type);

  return func.m_bValid;
}

void nsToolsReflectionUtils::GetReflectedTypeDescriptorFromRtti(const nsRTTI* pRtti, nsReflectedTypeDescriptor& out_desc)
{
  GetMinimalReflectedTypeDescriptorFromRtti(pRtti, out_desc);
  out_desc.m_Flags.Remove(nsTypeFlags::Minimal);

  auto rttiProps = pRtti->GetProperties();
  const nsUInt32 uiCount = rttiProps.GetCount();
  out_desc.m_Properties.Reserve(uiCount);
  for (nsUInt32 i = 0; i < uiCount; ++i)
  {
    const nsAbstractProperty* prop = rttiProps[i];

    switch (prop->GetCategory())
    {
      case nsPropertyCategory::Constant:
      {
        auto constantProp = static_cast<const nsAbstractConstantProperty*>(prop);
        const nsRTTI* pPropRtti = constantProp->GetSpecificType();
        if (nsReflectionUtils::IsBasicType(pPropRtti))
        {
          nsVariant value = constantProp->GetConstant();
          NS_ASSERT_DEV(pPropRtti->GetVariantType() == value.GetType(), "Variant value type and property type should always match!");
          out_desc.m_Properties.PushBack(nsReflectedPropertyDescriptor(constantProp->GetPropertyName(), value, prop->GetAttributes()));
        }
        else
        {
          NS_ASSERT_DEV(false, "Non-pod constants are not supported yet!");
        }
      }
      break;

      case nsPropertyCategory::Member:
      case nsPropertyCategory::Array:
      case nsPropertyCategory::Set:
      case nsPropertyCategory::Map:
      {
        const nsRTTI* pPropRtti = prop->GetSpecificType();
        out_desc.m_Properties.PushBack(nsReflectedPropertyDescriptor(prop->GetCategory(), prop->GetPropertyName(), pPropRtti->GetTypeName(), prop->GetFlags(), prop->GetAttributes()));
      }
      break;

      case nsPropertyCategory::Function:
        break;

      default:
        break;
    }
  }

  auto rttiFunc = pRtti->GetFunctions();
  const nsUInt32 uiFuncCount = rttiFunc.GetCount();
  out_desc.m_Functions.Reserve(uiFuncCount);

  for (nsUInt32 i = 0; i < uiFuncCount; ++i)
  {
    const nsAbstractFunctionProperty* prop = rttiFunc[i];
    out_desc.m_Functions.PushBack(nsReflectedFunctionDescriptor(prop->GetPropertyName(), prop->GetFlags(), prop->GetFunctionType(), prop->GetAttributes()));
    nsReflectedFunctionDescriptor& desc = out_desc.m_Functions.PeekBack();
    desc.m_ReturnValue = nsFunctionArgumentDescriptor(prop->GetReturnType() ? prop->GetReturnType()->GetTypeName() : "", prop->GetReturnFlags());
    const nsUInt32 uiArguments = prop->GetArgumentCount();
    desc.m_Arguments.Reserve(uiArguments);
    for (nsUInt32 a = 0; a < uiArguments; ++a)
    {
      desc.m_Arguments.PushBack(nsFunctionArgumentDescriptor(prop->GetArgumentType(a)->GetTypeName(), prop->GetArgumentFlags(a)));
    }
  }

  out_desc.m_ReferenceAttributes = pRtti->GetAttributes();
}


void nsToolsReflectionUtils::GetMinimalReflectedTypeDescriptorFromRtti(const nsRTTI* pRtti, nsReflectedTypeDescriptor& out_desc)
{
  NS_ASSERT_DEV(pRtti != nullptr, "Type to process must not be null!");
  out_desc.m_sTypeName = pRtti->GetTypeName();
  out_desc.m_sPluginName = pRtti->GetPluginName();
  out_desc.m_Flags = pRtti->GetTypeFlags() | nsTypeFlags::Minimal;
  out_desc.m_uiTypeVersion = pRtti->GetTypeVersion();
  const nsRTTI* pParentRtti = pRtti->GetParentType();
  out_desc.m_sParentTypeName = pParentRtti ? pParentRtti->GetTypeName() : nullptr;

  out_desc.m_Properties.Clear();
  out_desc.m_Functions.Clear();
  out_desc.m_Attributes.Clear();
  out_desc.m_ReferenceAttributes = nsArrayPtr<nsPropertyAttribute* const>();
}

static void GatherObjectTypesInternal(const nsDocumentObject* pObject, nsSet<const nsRTTI*>& inout_types)
{
  inout_types.Insert(pObject->GetTypeAccessor().GetType());
  nsReflectionUtils::GatherDependentTypes(pObject->GetTypeAccessor().GetType(), inout_types);

  for (const nsDocumentObject* pChild : pObject->GetChildren())
  {
    if (pChild->GetParentPropertyType()->GetAttributeByType<nsTemporaryAttribute>() != nullptr)
      continue;

    GatherObjectTypesInternal(pChild, inout_types);
  }
}

void nsToolsReflectionUtils::GatherObjectTypes(const nsDocumentObject* pObject, nsSet<const nsRTTI*>& inout_types)
{
  GatherObjectTypesInternal(pObject, inout_types);
}

bool nsToolsReflectionUtils::DependencySortTypeDescriptorArray(nsDynamicArray<nsReflectedTypeDescriptor*>& ref_descriptors)
{
  nsMap<nsReflectedTypeDescriptor*, nsSet<nsString>> dependencies;

  nsSet<nsString> typesInArray;
  // Gather all types in array
  for (nsReflectedTypeDescriptor* desc : ref_descriptors)
  {
    typesInArray.Insert(desc->m_sTypeName);
  }

  // Find all direct dependencies to types in the array for each type.
  for (nsReflectedTypeDescriptor* desc : ref_descriptors)
  {
    auto it = dependencies.Insert(desc, nsSet<nsString>());

    if (typesInArray.Contains(desc->m_sParentTypeName))
    {
      it.Value().Insert(desc->m_sParentTypeName);
    }
    for (nsReflectedPropertyDescriptor& propDesc : desc->m_Properties)
    {
      if (typesInArray.Contains(propDesc.m_sType))
      {
        it.Value().Insert(propDesc.m_sType);
      }
    }
  }

  nsSet<nsString> accu;
  nsDynamicArray<nsReflectedTypeDescriptor*> sorted;
  sorted.Reserve(ref_descriptors.GetCount());
  // Build new sorted types array.
  while (!ref_descriptors.IsEmpty())
  {
    bool bDeadEnd = true;
    for (nsReflectedTypeDescriptor* desc : ref_descriptors)
    {
      // Are the types dependencies met?
      if (accu.ContainsSet(dependencies[desc]))
      {
        sorted.PushBack(desc);
        bDeadEnd = false;
        ref_descriptors.RemoveAndCopy(desc);
        accu.Insert(desc->m_sTypeName);
        break;
      }
    }

    if (bDeadEnd)
    {
      return false;
    }
  }

  ref_descriptors = sorted;
  return true;
}
