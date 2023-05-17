#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Reflection/ReflectionUtils.h>
#include <ToolsFoundation/Reflection/PhantomProperty.h>
#include <ToolsFoundation/Reflection/PhantomRtti.h>

wdPhantomRTTI::wdPhantomRTTI(const char* szName, const wdRTTI* pParentType, wdUInt32 uiTypeSize, wdUInt32 uiTypeVersion, wdUInt32 uiVariantType,
  wdBitflags<wdTypeFlags> flags, const char* szPluginName)
  : wdRTTI(nullptr, pParentType, uiTypeSize, uiTypeVersion, uiVariantType, flags | wdTypeFlags::Phantom, nullptr, wdArrayPtr<wdAbstractProperty*>(),
      wdArrayPtr<wdAbstractProperty*>(), wdArrayPtr<wdPropertyAttribute*>(), wdArrayPtr<wdAbstractMessageHandler*>(),
      wdArrayPtr<wdMessageSenderInfo>(), nullptr)
{
  m_sTypeNameStorage = szName;
  m_sPluginNameStorage = szPluginName;

  m_szTypeName = m_sTypeNameStorage.GetData();
  m_szPluginName = m_sPluginNameStorage.GetData();

  RegisterType();
}

wdPhantomRTTI::~wdPhantomRTTI()
{
  UnregisterType();
  m_szTypeName = nullptr;

  for (auto pProp : m_PropertiesStorage)
  {
    WD_DEFAULT_DELETE(pProp);
  }
  for (auto pFunc : m_FunctionsStorage)
  {
    WD_DEFAULT_DELETE(pFunc);
  }
  for (auto pAttrib : m_AttributesStorage)
  {
    WD_DEFAULT_DELETE(pAttrib);
  }
}

void wdPhantomRTTI::SetProperties(wdDynamicArray<wdReflectedPropertyDescriptor>& properties)
{
  for (auto pProp : m_PropertiesStorage)
  {
    WD_DEFAULT_DELETE(pProp);
  }
  m_PropertiesStorage.Clear();

  const wdUInt32 iCount = properties.GetCount();
  m_PropertiesStorage.Reserve(iCount);

  for (wdUInt32 i = 0; i < iCount; i++)
  {
    switch (properties[i].m_Category)
    {
      case wdPropertyCategory::Constant:
      {
        m_PropertiesStorage.PushBack(WD_DEFAULT_NEW(wdPhantomConstantProperty, &properties[i]));
      }
      break;
      case wdPropertyCategory::Member:
      {
        m_PropertiesStorage.PushBack(WD_DEFAULT_NEW(wdPhantomMemberProperty, &properties[i]));
      }
      break;
      case wdPropertyCategory::Array:
      {
        m_PropertiesStorage.PushBack(WD_DEFAULT_NEW(wdPhantomArrayProperty, &properties[i]));
      }
      break;
      case wdPropertyCategory::Set:
      {
        m_PropertiesStorage.PushBack(WD_DEFAULT_NEW(wdPhantomSetProperty, &properties[i]));
      }
      break;
      case wdPropertyCategory::Map:
      {
        m_PropertiesStorage.PushBack(WD_DEFAULT_NEW(wdPhantomMapProperty, &properties[i]));
      }
      break;
      case wdPropertyCategory::Function:
        break; // Handled in SetFunctions
    }
  }

  m_Properties = m_PropertiesStorage;
}


void wdPhantomRTTI::SetFunctions(wdDynamicArray<wdReflectedFunctionDescriptor>& functions)
{
  for (auto pProp : m_FunctionsStorage)
  {
    WD_DEFAULT_DELETE(pProp);
  }
  m_FunctionsStorage.Clear();

  const wdUInt32 iCount = functions.GetCount();
  m_FunctionsStorage.Reserve(iCount);

  for (wdUInt32 i = 0; i < iCount; i++)
  {
    m_FunctionsStorage.PushBack(WD_DEFAULT_NEW(wdPhantomFunctionProperty, &functions[i]));
  }

  m_Functions = m_FunctionsStorage;
}

void wdPhantomRTTI::SetAttributes(wdHybridArray<wdPropertyAttribute*, 2>& attributes)
{
  for (auto pAttrib : m_AttributesStorage)
  {
    WD_DEFAULT_DELETE(pAttrib);
  }
  m_AttributesStorage.Clear();
  m_AttributesStorage = attributes;
  m_Attributes = m_AttributesStorage;
  attributes.Clear();
}

void wdPhantomRTTI::UpdateType(wdReflectedTypeDescriptor& desc)
{
  wdRTTI::UpdateType(wdRTTI::FindTypeByName(desc.m_sParentTypeName), 0, desc.m_uiTypeVersion, wdVariantType::Invalid, desc.m_Flags);

  m_sPluginNameStorage = desc.m_sPluginName;
  m_szPluginName = m_sPluginNameStorage.GetData();

  SetProperties(desc.m_Properties);
  SetFunctions(desc.m_Functions);
  SetAttributes(desc.m_Attributes);
  SetupParentHierarchy();
}

bool wdPhantomRTTI::IsEqualToDescriptor(const wdReflectedTypeDescriptor& desc)
{
  if ((desc.m_Flags.GetValue() & ~wdTypeFlags::Phantom) != (GetTypeFlags().GetValue() & ~wdTypeFlags::Phantom))
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

  for (wdUInt32 i = 0; i < GetProperties().GetCount(); i++)
  {
    if (desc.m_Properties[i].m_Category != GetProperties()[i]->GetCategory())
      return false;

    if (desc.m_Properties[i].m_sName != GetProperties()[i]->GetPropertyName())
      return false;

    if ((desc.m_Properties[i].m_Flags.GetValue() & ~wdPropertyFlags::Phantom) !=
        (GetProperties()[i]->GetFlags().GetValue() & ~wdPropertyFlags::Phantom))
      return false;

    switch (desc.m_Properties[i].m_Category)
    {
      case wdPropertyCategory::Constant:
      {
        auto pProp = (wdPhantomConstantProperty*)GetProperties()[i];

        if (pProp->GetSpecificType() != wdRTTI::FindTypeByName(desc.m_Properties[i].m_sType))
          return false;

        if (pProp->GetConstant() != desc.m_Properties[i].m_ConstantValue)
          return false;
      }
      break;
      case wdPropertyCategory::Member:
      {
        if (GetProperties()[i]->GetSpecificType() != wdRTTI::FindTypeByName(desc.m_Properties[i].m_sType))
          return false;
      }
      break;
      case wdPropertyCategory::Array:
      {
        if (GetProperties()[i]->GetSpecificType() != wdRTTI::FindTypeByName(desc.m_Properties[i].m_sType))
          return false;
      }
      break;
      case wdPropertyCategory::Set:
      {
        if (GetProperties()[i]->GetSpecificType() != wdRTTI::FindTypeByName(desc.m_Properties[i].m_sType))
          return false;
      }
      break;
      case wdPropertyCategory::Map:
      {
        if (GetProperties()[i]->GetSpecificType() != wdRTTI::FindTypeByName(desc.m_Properties[i].m_sType))
          return false;
      }
      break;
      case wdPropertyCategory::Function:
        break; // Functions handled below
    }

    if (desc.m_Functions.GetCount() != GetFunctions().GetCount())
      return false;

    for (wdUInt32 j = 0; j < GetFunctions().GetCount(); j++)
    {
      const wdAbstractFunctionProperty* pProp = GetFunctions()[j];
      if (desc.m_Functions[j].m_sName != pProp->GetPropertyName())
        return false;
      if ((desc.m_Functions[j].m_Flags.GetValue() & ~wdPropertyFlags::Phantom) != (pProp->GetFlags().GetValue() & ~wdPropertyFlags::Phantom))
        return false;
      if (desc.m_Functions[j].m_Type != pProp->GetFunctionType())
        return false;

      if (pProp->GetReturnType() != wdRTTI::FindTypeByName(desc.m_Functions[j].m_ReturnValue.m_sType))
        return false;
      if (pProp->GetReturnFlags() != desc.m_Functions[j].m_ReturnValue.m_Flags)
        return false;
      if (desc.m_Functions[j].m_Arguments.GetCount() != pProp->GetArgumentCount())
        return false;
      for (wdUInt32 a = 0; a < pProp->GetArgumentCount(); a++)
      {
        if (pProp->GetArgumentType(a) != wdRTTI::FindTypeByName(desc.m_Functions[j].m_Arguments[a].m_sType))
          return false;
        if (pProp->GetArgumentFlags(a) != desc.m_Functions[j].m_Arguments[a].m_Flags)
          return false;
      }
    }

    if (desc.m_Properties[i].m_Attributes.GetCount() != GetProperties()[i]->GetAttributes().GetCount())
      return false;

    for (wdUInt32 i2 = 0; i2 < desc.m_Properties[i].m_Attributes.GetCount(); i2++)
    {
      if (!wdReflectionUtils::IsEqual(desc.m_Properties[i].m_Attributes[i2], GetProperties()[i]->GetAttributes()[i2]))
        return false;
    }
  }

  if (desc.m_Attributes.GetCount() != GetAttributes().GetCount())
    return false;

  // TODO: compare attribute values?
  for (wdUInt32 i = 0; i < GetAttributes().GetCount(); i++)
  {
    if (desc.m_Attributes[i]->GetDynamicRTTI() != GetAttributes()[i]->GetDynamicRTTI())
      return false;
  }
  return true;
}
