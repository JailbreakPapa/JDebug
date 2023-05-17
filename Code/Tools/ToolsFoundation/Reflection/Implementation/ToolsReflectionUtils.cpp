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
    GetDoubleFunc(const wdVariant& value)
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

    const wdVariant& m_Value;
    double m_fValue = 0;
    bool m_bValid = false;
  };

  template <>
  void GetDoubleFunc::operator()<wdAngle>()
  {
    m_fValue = m_Value.Get<wdAngle>().GetDegree();
    m_bValid = true;
  }

  template <>
  void GetDoubleFunc::operator()<wdTime>()
  {
    m_fValue = m_Value.Get<wdTime>().GetSeconds();
    m_bValid = true;
  }

  struct GetVariantFunc
  {
    GetVariantFunc(double fValue, wdVariantType::Enum type, wdVariant& out_value)
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
        m_Value = wdVariant();
      }
    }

    double m_fValue;
    wdVariantType::Enum m_Type;
    wdVariant& m_Value;
    bool m_bValid = false;
  };

  template <>
  void GetVariantFunc::operator()<wdAngle>()
  {
    m_Value = wdAngle::Degree((float)m_fValue);
    m_bValid = true;
  }

  template <>
  void GetVariantFunc::operator()<wdTime>()
  {
    m_Value = wdTime::Seconds(m_fValue);
    m_bValid = true;
  }
} // namespace
////////////////////////////////////////////////////////////////////////
// wdToolsReflectionUtils public functions
////////////////////////////////////////////////////////////////////////

wdVariant wdToolsReflectionUtils::GetStorageDefault(const wdAbstractProperty* pProperty)
{
  const wdDefaultValueAttribute* pAttrib = pProperty->GetAttributeByType<wdDefaultValueAttribute>();
  auto type = pProperty->GetFlags().IsSet(wdPropertyFlags::StandardType) ? pProperty->GetSpecificType()->GetVariantType() : wdVariantType::Uuid;

  const bool bIsValueType = wdReflectionUtils::IsValueType(pProperty);

  switch (pProperty->GetCategory())
  {
    case wdPropertyCategory::Member:
    {
      return wdReflectionUtils::GetDefaultValue(pProperty);
    }
    break;
    case wdPropertyCategory::Array:
    case wdPropertyCategory::Set:
    {
      if (bIsValueType && pAttrib && pAttrib->GetValue().IsA<wdVariantArray>())
      {
        const wdVariantArray& value = pAttrib->GetValue().Get<wdVariantArray>();
        wdVariantArray ret;
        ret.SetCount(value.GetCount());
        for (wdUInt32 i = 0; i < value.GetCount(); i++)
        {
          ret[i] = value[i].ConvertTo(type);
        }
        return ret;
      }
      return wdVariantArray();
    }
    break;
    case wdPropertyCategory::Map:
    {
      return wdVariantDictionary();
    }
    break;
    case wdPropertyCategory::Constant:
    case wdPropertyCategory::Function:
      break; // no defaults
  }
  return wdVariant();
}

bool wdToolsReflectionUtils::GetFloatFromVariant(const wdVariant& val, double& out_fValue)
{
  if (val.IsValid())
  {
    GetDoubleFunc func(val);
    wdVariant::DispatchTo(func, val.GetType());
    out_fValue = func.m_fValue;
    return func.m_bValid;
  }
  return false;
}


bool wdToolsReflectionUtils::GetVariantFromFloat(double fValue, wdVariantType::Enum type, wdVariant& out_val)
{
  GetVariantFunc func(fValue, type, out_val);
  wdVariant::DispatchTo(func, type);

  return func.m_bValid;
}

void wdToolsReflectionUtils::GetReflectedTypeDescriptorFromRtti(const wdRTTI* pRtti, wdReflectedTypeDescriptor& out_desc)
{
  GetMinimalReflectedTypeDescriptorFromRtti(pRtti, out_desc);
  out_desc.m_Flags.Remove(wdTypeFlags::Minimal);

  const wdArrayPtr<wdAbstractProperty*>& rttiProps = pRtti->GetProperties();
  const wdUInt32 uiCount = rttiProps.GetCount();
  out_desc.m_Properties.Reserve(uiCount);
  for (wdUInt32 i = 0; i < uiCount; ++i)
  {
    wdAbstractProperty* prop = rttiProps[i];

    switch (prop->GetCategory())
    {
      case wdPropertyCategory::Constant:
      {
        wdAbstractConstantProperty* constantProp = static_cast<wdAbstractConstantProperty*>(prop);
        const wdRTTI* pPropRtti = constantProp->GetSpecificType();
        if (wdReflectionUtils::IsBasicType(pPropRtti))
        {
          wdVariant value = constantProp->GetConstant();
          WD_ASSERT_DEV(pPropRtti->GetVariantType() == value.GetType(), "Variant value type and property type should always match!");
          out_desc.m_Properties.PushBack(wdReflectedPropertyDescriptor(constantProp->GetPropertyName(), value, prop->GetAttributes()));
        }
        else
        {
          WD_ASSERT_DEV(false, "Non-pod constants are not supported yet!");
        }
      }
      break;

      case wdPropertyCategory::Member:
      case wdPropertyCategory::Array:
      case wdPropertyCategory::Set:
      case wdPropertyCategory::Map:
      {
        const wdRTTI* pPropRtti = prop->GetSpecificType();
        out_desc.m_Properties.PushBack(wdReflectedPropertyDescriptor(
          prop->GetCategory(), prop->GetPropertyName(), pPropRtti->GetTypeName(), prop->GetFlags(), prop->GetAttributes()));
      }
      break;

      case wdPropertyCategory::Function:
        break;

      default:
        break;
    }
  }

  const wdArrayPtr<wdAbstractFunctionProperty*>& rttiFunc = pRtti->GetFunctions();
  const wdUInt32 uiFuncCount = rttiFunc.GetCount();
  out_desc.m_Functions.Reserve(uiFuncCount);

  for (wdUInt32 i = 0; i < uiFuncCount; ++i)
  {
    wdAbstractFunctionProperty* prop = rttiFunc[i];
    out_desc.m_Functions.PushBack(
      wdReflectedFunctionDescriptor(prop->GetPropertyName(), prop->GetFlags(), prop->GetFunctionType(), prop->GetAttributes()));
    wdReflectedFunctionDescriptor& desc = out_desc.m_Functions.PeekBack();
    desc.m_ReturnValue = wdFunctionArgumentDescriptor(prop->GetReturnType() ? prop->GetReturnType()->GetTypeName() : "", prop->GetReturnFlags());
    const wdUInt32 uiArguments = prop->GetArgumentCount();
    desc.m_Arguments.Reserve(uiArguments);
    for (wdUInt32 a = 0; a < uiArguments; ++a)
    {
      desc.m_Arguments.PushBack(wdFunctionArgumentDescriptor(prop->GetArgumentType(a)->GetTypeName(), prop->GetArgumentFlags(a)));
    }
  }

  out_desc.m_ReferenceAttributes = pRtti->GetAttributes();
}


void wdToolsReflectionUtils::GetMinimalReflectedTypeDescriptorFromRtti(const wdRTTI* pRtti, wdReflectedTypeDescriptor& out_desc)
{
  WD_ASSERT_DEV(pRtti != nullptr, "Type to process must not be null!");
  out_desc.m_sTypeName = pRtti->GetTypeName();
  out_desc.m_sPluginName = pRtti->GetPluginName();
  out_desc.m_Flags = pRtti->GetTypeFlags() | wdTypeFlags::Minimal;
  out_desc.m_uiTypeVersion = pRtti->GetTypeVersion();
  const wdRTTI* pParentRtti = pRtti->GetParentType();
  out_desc.m_sParentTypeName = pParentRtti ? pParentRtti->GetTypeName() : nullptr;

  out_desc.m_Properties.Clear();
  out_desc.m_Functions.Clear();
  out_desc.m_Attributes.Clear();
  out_desc.m_ReferenceAttributes = wdArrayPtr<wdPropertyAttribute* const>();
}

static void GatherObjectTypesInternal(const wdDocumentObject* pObject, wdSet<const wdRTTI*>& inout_types)
{
  inout_types.Insert(pObject->GetTypeAccessor().GetType());
  wdReflectionUtils::GatherDependentTypes(pObject->GetTypeAccessor().GetType(), inout_types);

  for (const wdDocumentObject* pChild : pObject->GetChildren())
  {
    if (pChild->GetParentPropertyType()->GetAttributeByType<wdTemporaryAttribute>() != nullptr)
      continue;

    GatherObjectTypesInternal(pChild, inout_types);
  }
}

void wdToolsReflectionUtils::GatherObjectTypes(const wdDocumentObject* pObject, wdSet<const wdRTTI*>& inout_types)
{
  GatherObjectTypesInternal(pObject, inout_types);
}

bool wdToolsReflectionUtils::DependencySortTypeDescriptorArray(wdDynamicArray<wdReflectedTypeDescriptor*>& ref_descriptors)
{
  wdMap<wdReflectedTypeDescriptor*, wdSet<wdString>> dependencies;

  wdSet<wdString> typesInArray;
  // Gather all types in array
  for (wdReflectedTypeDescriptor* desc : ref_descriptors)
  {
    typesInArray.Insert(desc->m_sTypeName);
  }

  // Find all direct dependencies to types in the array for each type.
  for (wdReflectedTypeDescriptor* desc : ref_descriptors)
  {
    auto it = dependencies.Insert(desc, wdSet<wdString>());

    if (typesInArray.Contains(desc->m_sParentTypeName))
    {
      it.Value().Insert(desc->m_sParentTypeName);
    }
    for (wdReflectedPropertyDescriptor& propDesc : desc->m_Properties)
    {
      if (typesInArray.Contains(propDesc.m_sType))
      {
        it.Value().Insert(propDesc.m_sType);
      }
    }
  }

  wdSet<wdString> accu;
  wdDynamicArray<wdReflectedTypeDescriptor*> sorted;
  sorted.Reserve(ref_descriptors.GetCount());
  // Build new sorted types array.
  while (!ref_descriptors.IsEmpty())
  {
    bool bDeadEnd = true;
    for (wdReflectedTypeDescriptor* desc : ref_descriptors)
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
