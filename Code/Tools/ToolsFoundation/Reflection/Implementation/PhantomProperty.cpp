#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Reflection/PhantomProperty.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>

wdPhantomConstantProperty::wdPhantomConstantProperty(const wdReflectedPropertyDescriptor* pDesc)
  : wdAbstractConstantProperty(nullptr)
{
  m_sPropertyNameStorage = pDesc->m_sName;
  m_szPropertyName = m_sPropertyNameStorage.GetData();
  m_Value = pDesc->m_ConstantValue;
  m_pPropertyType = wdRTTI::FindTypeByName(pDesc->m_sType);

  m_Flags = pDesc->m_Flags;
  m_Flags.Add(wdPropertyFlags::Phantom);
  m_Attributes = pDesc->m_Attributes;
  pDesc->m_Attributes.Clear();
}

wdPhantomConstantProperty::~wdPhantomConstantProperty()
{
  for (auto pAttr : m_Attributes)
    pAttr->GetDynamicRTTI()->GetAllocator()->Deallocate(pAttr);
}

const wdRTTI* wdPhantomConstantProperty::GetSpecificType() const
{
  return m_pPropertyType;
}

void* wdPhantomConstantProperty::GetPropertyPointer() const
{
  return nullptr;
}



wdPhantomMemberProperty::wdPhantomMemberProperty(const wdReflectedPropertyDescriptor* pDesc)
  : wdAbstractMemberProperty(nullptr)
{
  m_sPropertyNameStorage = pDesc->m_sName;
  m_szPropertyName = m_sPropertyNameStorage.GetData();
  m_pPropertyType = wdRTTI::FindTypeByName(pDesc->m_sType);

  m_Flags = pDesc->m_Flags;
  m_Flags.Add(wdPropertyFlags::Phantom);
  m_Attributes = pDesc->m_Attributes;
  pDesc->m_Attributes.Clear();
}

wdPhantomMemberProperty::~wdPhantomMemberProperty()
{
  for (auto pAttr : m_Attributes)
    pAttr->GetDynamicRTTI()->GetAllocator()->Deallocate(pAttr);
}

const wdRTTI* wdPhantomMemberProperty::GetSpecificType() const
{
  return m_pPropertyType;
}



wdPhantomFunctionProperty::wdPhantomFunctionProperty(wdReflectedFunctionDescriptor* pDesc)
  : wdAbstractFunctionProperty(nullptr)
{
  m_sPropertyNameStorage = pDesc->m_sName;
  m_szPropertyName = m_sPropertyNameStorage.GetData();
  m_FunctionType = pDesc->m_Type;
  m_Flags = pDesc->m_Flags;
  m_Flags.Add(wdPropertyFlags::Phantom);
  m_Attributes = pDesc->m_Attributes;
  pDesc->m_Attributes.Clear();

  m_ReturnValue = pDesc->m_ReturnValue;
  m_Arguments.Swap(pDesc->m_Arguments);
}



wdPhantomFunctionProperty::~wdPhantomFunctionProperty()
{
  for (auto pAttr : m_Attributes)
    pAttr->GetDynamicRTTI()->GetAllocator()->Deallocate(pAttr);
}

wdFunctionType::Enum wdPhantomFunctionProperty::GetFunctionType() const
{
  return m_FunctionType;
}

const wdRTTI* wdPhantomFunctionProperty::GetReturnType() const
{
  return wdRTTI::FindTypeByName(m_ReturnValue.m_sType);
}

wdBitflags<wdPropertyFlags> wdPhantomFunctionProperty::GetReturnFlags() const
{
  return m_ReturnValue.m_Flags;
}

wdUInt32 wdPhantomFunctionProperty::GetArgumentCount() const
{
  return m_Arguments.GetCount();
}

const wdRTTI* wdPhantomFunctionProperty::GetArgumentType(wdUInt32 uiParamIndex) const
{
  return wdRTTI::FindTypeByName(m_Arguments[uiParamIndex].m_sType);
}

wdBitflags<wdPropertyFlags> wdPhantomFunctionProperty::GetArgumentFlags(wdUInt32 uiParamIndex) const
{
  return m_Arguments[uiParamIndex].m_Flags;
}

void wdPhantomFunctionProperty::Execute(void* pInstance, wdArrayPtr<wdVariant> values, wdVariant& ref_returnValue) const
{
  WD_ASSERT_NOT_IMPLEMENTED;
}

wdPhantomArrayProperty::wdPhantomArrayProperty(const wdReflectedPropertyDescriptor* pDesc)
  : wdAbstractArrayProperty(nullptr)
{
  m_sPropertyNameStorage = pDesc->m_sName;
  m_szPropertyName = m_sPropertyNameStorage.GetData();
  m_pPropertyType = wdRTTI::FindTypeByName(pDesc->m_sType);

  m_Flags = pDesc->m_Flags;
  m_Flags.Add(wdPropertyFlags::Phantom);
  m_Attributes = pDesc->m_Attributes;
  pDesc->m_Attributes.Clear();
}

wdPhantomArrayProperty::~wdPhantomArrayProperty()
{
  for (auto pAttr : m_Attributes)
    pAttr->GetDynamicRTTI()->GetAllocator()->Deallocate(pAttr);
}

const wdRTTI* wdPhantomArrayProperty::GetSpecificType() const
{
  return m_pPropertyType;
}

wdPhantomSetProperty::wdPhantomSetProperty(const wdReflectedPropertyDescriptor* pDesc)
  : wdAbstractSetProperty(nullptr)
{
  m_sPropertyNameStorage = pDesc->m_sName;
  m_szPropertyName = m_sPropertyNameStorage.GetData();
  m_pPropertyType = wdRTTI::FindTypeByName(pDesc->m_sType);

  m_Flags = pDesc->m_Flags;
  m_Flags.Add(wdPropertyFlags::Phantom);
  m_Attributes = pDesc->m_Attributes;
  pDesc->m_Attributes.Clear();
}

wdPhantomSetProperty::~wdPhantomSetProperty()
{
  for (auto pAttr : m_Attributes)
    pAttr->GetDynamicRTTI()->GetAllocator()->Deallocate(pAttr);
}

const wdRTTI* wdPhantomSetProperty::GetSpecificType() const
{
  return m_pPropertyType;
}

wdPhantomMapProperty::wdPhantomMapProperty(const wdReflectedPropertyDescriptor* pDesc)
  : wdAbstractMapProperty(nullptr)
{
  m_sPropertyNameStorage = pDesc->m_sName;
  m_szPropertyName = m_sPropertyNameStorage.GetData();
  m_pPropertyType = wdRTTI::FindTypeByName(pDesc->m_sType);

  m_Flags = pDesc->m_Flags;
  m_Flags.Add(wdPropertyFlags::Phantom);
  m_Attributes = pDesc->m_Attributes;
  pDesc->m_Attributes.Clear();
}

wdPhantomMapProperty::~wdPhantomMapProperty()
{
  for (auto pAttr : m_Attributes)
    pAttr->GetDynamicRTTI()->GetAllocator()->Deallocate(pAttr);
}

const wdRTTI* wdPhantomMapProperty::GetSpecificType() const
{
  return m_pPropertyType;
}
