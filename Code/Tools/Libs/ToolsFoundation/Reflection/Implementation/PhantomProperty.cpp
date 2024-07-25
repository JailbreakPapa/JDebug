#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Reflection/PhantomProperty.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>

nsPhantomConstantProperty::nsPhantomConstantProperty(const nsReflectedPropertyDescriptor* pDesc)
  : nsAbstractConstantProperty(nullptr)
{
  m_sPropertyNameStorage = pDesc->m_sName;
  m_szPropertyName = m_sPropertyNameStorage.GetData();
  m_Value = pDesc->m_ConstantValue;
  m_pPropertyType = nsRTTI::FindTypeByName(pDesc->m_sType);

  m_Flags = pDesc->m_Flags;
  m_Flags.Add(nsPropertyFlags::Phantom);
  m_Attributes = pDesc->m_Attributes;
  pDesc->m_Attributes.Clear();
}

nsPhantomConstantProperty::~nsPhantomConstantProperty()
{
  for (auto pAttr : m_Attributes)
  {
    pAttr->GetDynamicRTTI()->GetAllocator()->Deallocate(const_cast<nsPropertyAttribute*>(pAttr));
  }
}

const nsRTTI* nsPhantomConstantProperty::GetSpecificType() const
{
  return m_pPropertyType;
}

void* nsPhantomConstantProperty::GetPropertyPointer() const
{
  return nullptr;
}



nsPhantomMemberProperty::nsPhantomMemberProperty(const nsReflectedPropertyDescriptor* pDesc)
  : nsAbstractMemberProperty(nullptr)
{
  m_sPropertyNameStorage = pDesc->m_sName;
  m_szPropertyName = m_sPropertyNameStorage.GetData();
  m_pPropertyType = nsRTTI::FindTypeByName(pDesc->m_sType);

  m_Flags = pDesc->m_Flags;
  m_Flags.Add(nsPropertyFlags::Phantom);
  m_Attributes = pDesc->m_Attributes;
  pDesc->m_Attributes.Clear();
}

nsPhantomMemberProperty::~nsPhantomMemberProperty()
{
  for (auto pAttr : m_Attributes)
  {
    pAttr->GetDynamicRTTI()->GetAllocator()->Deallocate(const_cast<nsPropertyAttribute*>(pAttr));
  }
}

const nsRTTI* nsPhantomMemberProperty::GetSpecificType() const
{
  return m_pPropertyType;
}



nsPhantomFunctionProperty::nsPhantomFunctionProperty(nsReflectedFunctionDescriptor* pDesc)
  : nsAbstractFunctionProperty(nullptr)
{
  m_sPropertyNameStorage = pDesc->m_sName;
  m_szPropertyName = m_sPropertyNameStorage.GetData();
  m_FunctionType = pDesc->m_Type;
  m_Flags = pDesc->m_Flags;
  m_Flags.Add(nsPropertyFlags::Phantom);
  m_Attributes = pDesc->m_Attributes;
  pDesc->m_Attributes.Clear();

  m_ReturnValue = pDesc->m_ReturnValue;
  m_Arguments.Swap(pDesc->m_Arguments);
}



nsPhantomFunctionProperty::~nsPhantomFunctionProperty()
{
  for (auto pAttr : m_Attributes)
  {
    pAttr->GetDynamicRTTI()->GetAllocator()->Deallocate(const_cast<nsPropertyAttribute*>(pAttr));
  }
}

nsFunctionType::Enum nsPhantomFunctionProperty::GetFunctionType() const
{
  return m_FunctionType;
}

const nsRTTI* nsPhantomFunctionProperty::GetReturnType() const
{
  return nsRTTI::FindTypeByName(m_ReturnValue.m_sType);
}

nsBitflags<nsPropertyFlags> nsPhantomFunctionProperty::GetReturnFlags() const
{
  return m_ReturnValue.m_Flags;
}

nsUInt32 nsPhantomFunctionProperty::GetArgumentCount() const
{
  return m_Arguments.GetCount();
}

const nsRTTI* nsPhantomFunctionProperty::GetArgumentType(nsUInt32 uiParamIndex) const
{
  return nsRTTI::FindTypeByName(m_Arguments[uiParamIndex].m_sType);
}

nsBitflags<nsPropertyFlags> nsPhantomFunctionProperty::GetArgumentFlags(nsUInt32 uiParamIndex) const
{
  return m_Arguments[uiParamIndex].m_Flags;
}

void nsPhantomFunctionProperty::Execute(void* pInstance, nsArrayPtr<nsVariant> values, nsVariant& ref_returnValue) const
{
  NS_ASSERT_NOT_IMPLEMENTED;
}

nsPhantomArrayProperty::nsPhantomArrayProperty(const nsReflectedPropertyDescriptor* pDesc)
  : nsAbstractArrayProperty(nullptr)
{
  m_sPropertyNameStorage = pDesc->m_sName;
  m_szPropertyName = m_sPropertyNameStorage.GetData();
  m_pPropertyType = nsRTTI::FindTypeByName(pDesc->m_sType);

  m_Flags = pDesc->m_Flags;
  m_Flags.Add(nsPropertyFlags::Phantom);
  m_Attributes = pDesc->m_Attributes;
  pDesc->m_Attributes.Clear();
}

nsPhantomArrayProperty::~nsPhantomArrayProperty()
{
  for (auto pAttr : m_Attributes)
  {
    pAttr->GetDynamicRTTI()->GetAllocator()->Deallocate(const_cast<nsPropertyAttribute*>(pAttr));
  }
}

const nsRTTI* nsPhantomArrayProperty::GetSpecificType() const
{
  return m_pPropertyType;
}

nsPhantomSetProperty::nsPhantomSetProperty(const nsReflectedPropertyDescriptor* pDesc)
  : nsAbstractSetProperty(nullptr)
{
  m_sPropertyNameStorage = pDesc->m_sName;
  m_szPropertyName = m_sPropertyNameStorage.GetData();
  m_pPropertyType = nsRTTI::FindTypeByName(pDesc->m_sType);

  m_Flags = pDesc->m_Flags;
  m_Flags.Add(nsPropertyFlags::Phantom);
  m_Attributes = pDesc->m_Attributes;
  pDesc->m_Attributes.Clear();
}

nsPhantomSetProperty::~nsPhantomSetProperty()
{
  for (auto pAttr : m_Attributes)
  {
    pAttr->GetDynamicRTTI()->GetAllocator()->Deallocate(const_cast<nsPropertyAttribute*>(pAttr));
  }
}

const nsRTTI* nsPhantomSetProperty::GetSpecificType() const
{
  return m_pPropertyType;
}

nsPhantomMapProperty::nsPhantomMapProperty(const nsReflectedPropertyDescriptor* pDesc)
  : nsAbstractMapProperty(nullptr)
{
  m_sPropertyNameStorage = pDesc->m_sName;
  m_szPropertyName = m_sPropertyNameStorage.GetData();
  m_pPropertyType = nsRTTI::FindTypeByName(pDesc->m_sType);

  m_Flags = pDesc->m_Flags;
  m_Flags.Add(nsPropertyFlags::Phantom);
  m_Attributes = pDesc->m_Attributes;
  pDesc->m_Attributes.Clear();
}

nsPhantomMapProperty::~nsPhantomMapProperty()
{
  for (auto pAttr : m_Attributes)
  {
    pAttr->GetDynamicRTTI()->GetAllocator()->Deallocate(const_cast<nsPropertyAttribute*>(pAttr));
  }
}

const nsRTTI* nsPhantomMapProperty::GetSpecificType() const
{
  return m_pPropertyType;
}
