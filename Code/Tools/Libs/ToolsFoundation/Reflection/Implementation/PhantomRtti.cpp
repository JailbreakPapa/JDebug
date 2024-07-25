#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Reflection/ReflectionUtils.h>
#include <ToolsFoundation/Reflection/PhantomProperty.h>
#include <ToolsFoundation/Reflection/PhantomRtti.h>

nsPhantomRTTI::nsPhantomRTTI(nsStringView sName, const nsRTTI* pParentType, nsUInt32 uiTypeSize, nsUInt32 uiTypeVersion, nsUInt8 uiVariantType,
  nsBitflags<nsTypeFlags> flags, nsStringView sPluginName)
  : nsRTTI(nullptr, pParentType, uiTypeSize, uiTypeVersion, uiVariantType, flags | nsTypeFlags::Phantom, nullptr, nsArrayPtr<const nsAbstractProperty*>(),
      nsArrayPtr<const nsAbstractFunctionProperty*>(), nsArrayPtr<const nsPropertyAttribute*>(), nsArrayPtr<nsAbstractMessageHandler*>(),
      nsArrayPtr<nsMessageSenderInfo>(), nullptr)
{
  m_sTypeNameStorage = sName;
  m_sPluginNameStorage = sPluginName;

  m_sTypeName = m_sTypeNameStorage.GetData();
  m_sPluginName = m_sPluginNameStorage.GetData();

  RegisterType();
}

nsPhantomRTTI::~nsPhantomRTTI()
{
  UnregisterType();
  m_sTypeName = nullptr;

  for (auto pProp : m_PropertiesStorage)
  {
    NS_DEFAULT_DELETE(pProp);
  }
  for (auto pFunc : m_FunctionsStorage)
  {
    NS_DEFAULT_DELETE(pFunc);
  }
  for (auto pAttrib : m_AttributesStorage)
  {
    auto pAttribNonConst = const_cast<nsPropertyAttribute*>(pAttrib);
    NS_DEFAULT_DELETE(pAttribNonConst);
  }
}

void nsPhantomRTTI::SetProperties(nsDynamicArray<nsReflectedPropertyDescriptor>& properties)
{
  for (auto pProp : m_PropertiesStorage)
  {
    NS_DEFAULT_DELETE(pProp);
  }
  m_PropertiesStorage.Clear();

  const nsUInt32 iCount = properties.GetCount();
  m_PropertiesStorage.Reserve(iCount);

  for (nsUInt32 i = 0; i < iCount; i++)
  {
    switch (properties[i].m_Category)
    {
      case nsPropertyCategory::Constant:
      {
        m_PropertiesStorage.PushBack(NS_DEFAULT_NEW(nsPhantomConstantProperty, &properties[i]));
      }
      break;
      case nsPropertyCategory::Member:
      {
        m_PropertiesStorage.PushBack(NS_DEFAULT_NEW(nsPhantomMemberProperty, &properties[i]));
      }
      break;
      case nsPropertyCategory::Array:
      {
        m_PropertiesStorage.PushBack(NS_DEFAULT_NEW(nsPhantomArrayProperty, &properties[i]));
      }
      break;
      case nsPropertyCategory::Set:
      {
        m_PropertiesStorage.PushBack(NS_DEFAULT_NEW(nsPhantomSetProperty, &properties[i]));
      }
      break;
      case nsPropertyCategory::Map:
      {
        m_PropertiesStorage.PushBack(NS_DEFAULT_NEW(nsPhantomMapProperty, &properties[i]));
      }
      break;
      case nsPropertyCategory::Function:
        break; // Handled in SetFunctions
    }
  }

  m_Properties = m_PropertiesStorage.GetArrayPtr();
}


void nsPhantomRTTI::SetFunctions(nsDynamicArray<nsReflectedFunctionDescriptor>& functions)
{
  for (auto pProp : m_FunctionsStorage)
  {
    NS_DEFAULT_DELETE(pProp);
  }
  m_FunctionsStorage.Clear();

  const nsUInt32 iCount = functions.GetCount();
  m_FunctionsStorage.Reserve(iCount);

  for (nsUInt32 i = 0; i < iCount; i++)
  {
    m_FunctionsStorage.PushBack(NS_DEFAULT_NEW(nsPhantomFunctionProperty, &functions[i]));
  }

  m_Functions = m_FunctionsStorage.GetArrayPtr();
}

void nsPhantomRTTI::SetAttributes(nsDynamicArray<const nsPropertyAttribute*>& attributes)
{
  for (auto pAttrib : m_AttributesStorage)
  {
    auto pAttribNonConst = const_cast<nsPropertyAttribute*>(pAttrib);
    NS_DEFAULT_DELETE(pAttribNonConst);
  }
  m_AttributesStorage.Clear();
  m_AttributesStorage = attributes;
  m_Attributes = m_AttributesStorage;
  attributes.Clear();
}

void nsPhantomRTTI::UpdateType(nsReflectedTypeDescriptor& desc)
{
  nsRTTI::UpdateType(nsRTTI::FindTypeByName(desc.m_sParentTypeName), 0, desc.m_uiTypeVersion, nsVariantType::Invalid, desc.m_Flags);

  m_sPluginNameStorage = desc.m_sPluginName;
  m_sPluginName = m_sPluginNameStorage.GetData();

  SetProperties(desc.m_Properties);
  SetFunctions(desc.m_Functions);
  SetAttributes(desc.m_Attributes);
  SetupParentHierarchy();
}

bool nsPhantomRTTI::IsEqualToDescriptor(const nsReflectedTypeDescriptor& desc)
{
  if ((desc.m_Flags.GetValue() & ~nsTypeFlags::Phantom) != (GetTypeFlags().GetValue() & ~nsTypeFlags::Phantom))
    return false;

  if (desc.m_sParentTypeName.IsEmpty() && GetParentType() != nullptr)
    return false;

  if (GetParentType() != nullptr && desc.m_sParentTypeName != GetParentType()->GetTypeName())
    return false;

  if (desc.m_sPluginName != GetPluginName())
    return false;

  if (desc.m_sTypeName != GetTypeName())
    return false;

  if (desc.m_Properties.GetCount() != GetProperties().GetCount())
    return false;

  for (nsUInt32 i = 0; i < GetProperties().GetCount(); i++)
  {
    if (desc.m_Properties[i].m_Category != GetProperties()[i]->GetCategory())
      return false;

    if (desc.m_Properties[i].m_sName != GetProperties()[i]->GetPropertyName())
      return false;

    if ((desc.m_Properties[i].m_Flags.GetValue() & ~nsPropertyFlags::Phantom) !=
        (GetProperties()[i]->GetFlags().GetValue() & ~nsPropertyFlags::Phantom))
      return false;

    switch (desc.m_Properties[i].m_Category)
    {
      case nsPropertyCategory::Constant:
      {
        auto pProp = (nsPhantomConstantProperty*)GetProperties()[i];

        if (pProp->GetSpecificType() != nsRTTI::FindTypeByName(desc.m_Properties[i].m_sType))
          return false;

        if (pProp->GetConstant() != desc.m_Properties[i].m_ConstantValue)
          return false;
      }
      break;
      case nsPropertyCategory::Member:
      {
        if (GetProperties()[i]->GetSpecificType() != nsRTTI::FindTypeByName(desc.m_Properties[i].m_sType))
          return false;
      }
      break;
      case nsPropertyCategory::Array:
      {
        if (GetProperties()[i]->GetSpecificType() != nsRTTI::FindTypeByName(desc.m_Properties[i].m_sType))
          return false;
      }
      break;
      case nsPropertyCategory::Set:
      {
        if (GetProperties()[i]->GetSpecificType() != nsRTTI::FindTypeByName(desc.m_Properties[i].m_sType))
          return false;
      }
      break;
      case nsPropertyCategory::Map:
      {
        if (GetProperties()[i]->GetSpecificType() != nsRTTI::FindTypeByName(desc.m_Properties[i].m_sType))
          return false;
      }
      break;
      case nsPropertyCategory::Function:
        break; // Functions handled below
    }

    if (desc.m_Functions.GetCount() != GetFunctions().GetCount())
      return false;

    for (nsUInt32 j = 0; j < GetFunctions().GetCount(); j++)
    {
      const nsAbstractFunctionProperty* pProp = GetFunctions()[j];
      if (desc.m_Functions[j].m_sName != pProp->GetPropertyName())
        return false;
      if ((desc.m_Functions[j].m_Flags.GetValue() & ~nsPropertyFlags::Phantom) != (pProp->GetFlags().GetValue() & ~nsPropertyFlags::Phantom))
        return false;
      if (desc.m_Functions[j].m_Type != pProp->GetFunctionType())
        return false;

      if (pProp->GetReturnType() != nsRTTI::FindTypeByName(desc.m_Functions[j].m_ReturnValue.m_sType))
        return false;
      if (pProp->GetReturnFlags() != desc.m_Functions[j].m_ReturnValue.m_Flags)
        return false;
      if (desc.m_Functions[j].m_Arguments.GetCount() != pProp->GetArgumentCount())
        return false;
      for (nsUInt32 a = 0; a < pProp->GetArgumentCount(); a++)
      {
        if (pProp->GetArgumentType(a) != nsRTTI::FindTypeByName(desc.m_Functions[j].m_Arguments[a].m_sType))
          return false;
        if (pProp->GetArgumentFlags(a) != desc.m_Functions[j].m_Arguments[a].m_Flags)
          return false;
      }
    }

    if (desc.m_Properties[i].m_Attributes.GetCount() != GetProperties()[i]->GetAttributes().GetCount())
      return false;

    for (nsUInt32 i2 = 0; i2 < desc.m_Properties[i].m_Attributes.GetCount(); i2++)
    {
      if (!nsReflectionUtils::IsEqual(desc.m_Properties[i].m_Attributes[i2], GetProperties()[i]->GetAttributes()[i2]))
        return false;
    }
  }

  if (desc.m_Attributes.GetCount() != GetAttributes().GetCount())
    return false;

  // TODO: compare attribute values?
  for (nsUInt32 i = 0; i < GetAttributes().GetCount(); i++)
  {
    if (desc.m_Attributes[i]->GetDynamicRTTI() != GetAttributes()[i]->GetDynamicRTTI())
      return false;
  }
  return true;
}
