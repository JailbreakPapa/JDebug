#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Object/ObjectProxyAccessor.h>

nsObjectProxyAccessor::nsObjectProxyAccessor(nsObjectAccessorBase* pSource)
  : nsObjectAccessorBase(pSource->GetObjectManager())
  , m_pSource(pSource)
{
}

nsObjectProxyAccessor::~nsObjectProxyAccessor() = default;

void nsObjectProxyAccessor::StartTransaction(nsStringView sDisplayString)
{
  m_pSource->StartTransaction(sDisplayString);
}

void nsObjectProxyAccessor::CancelTransaction()
{
  m_pSource->CancelTransaction();
}

void nsObjectProxyAccessor::FinishTransaction()
{
  m_pSource->FinishTransaction();
}

void nsObjectProxyAccessor::BeginTemporaryCommands(nsStringView sDisplayString, bool bFireEventsWhenUndoingTempCommands /*= false*/)
{
  m_pSource->BeginTemporaryCommands(sDisplayString, bFireEventsWhenUndoingTempCommands);
}

void nsObjectProxyAccessor::CancelTemporaryCommands()
{
  m_pSource->CancelTemporaryCommands();
}

void nsObjectProxyAccessor::FinishTemporaryCommands()
{
  m_pSource->FinishTemporaryCommands();
}

const nsDocumentObject* nsObjectProxyAccessor::GetObject(const nsUuid& object)
{
  return m_pSource->GetObject(object);
}

nsStatus nsObjectProxyAccessor::GetValue(
  const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant& out_value, nsVariant index /*= nsVariant()*/)
{
  return m_pSource->GetValue(pObject, pProp, out_value, index);
}

nsStatus nsObjectProxyAccessor::SetValue(
  const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& newValue, nsVariant index /*= nsVariant()*/)
{
  return m_pSource->SetValue(pObject, pProp, newValue, index);
}

nsStatus nsObjectProxyAccessor::InsertValue(
  const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& newValue, nsVariant index /*= nsVariant()*/)
{
  return m_pSource->InsertValue(pObject, pProp, newValue, index);
}

nsStatus nsObjectProxyAccessor::RemoveValue(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant index /*= nsVariant()*/)
{
  return m_pSource->RemoveValue(pObject, pProp, index);
}

nsStatus nsObjectProxyAccessor::MoveValue(
  const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& oldIndex, const nsVariant& newIndex)
{
  return m_pSource->MoveValue(pObject, pProp, oldIndex, newIndex);
}

nsStatus nsObjectProxyAccessor::GetCount(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsInt32& out_iCount)
{
  return m_pSource->GetCount(pObject, pProp, out_iCount);
}

nsStatus nsObjectProxyAccessor::AddObject(
  const nsDocumentObject* pParent, const nsAbstractProperty* pParentProp, const nsVariant& index, const nsRTTI* pType, nsUuid& inout_objectGuid)
{
  return m_pSource->AddObject(pParent, pParentProp, index, pType, inout_objectGuid);
}

nsStatus nsObjectProxyAccessor::RemoveObject(const nsDocumentObject* pObject)
{
  return m_pSource->RemoveObject(pObject);
}

nsStatus nsObjectProxyAccessor::MoveObject(
  const nsDocumentObject* pObject, const nsDocumentObject* pNewParent, const nsAbstractProperty* pParentProp, const nsVariant& index)
{
  return m_pSource->MoveObject(pObject, pNewParent, pParentProp, index);
}

nsStatus nsObjectProxyAccessor::GetKeys(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsDynamicArray<nsVariant>& out_keys)
{
  return m_pSource->GetKeys(pObject, pProp, out_keys);
}

nsStatus nsObjectProxyAccessor::GetValues(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsDynamicArray<nsVariant>& out_values)
{
  return m_pSource->GetValues(pObject, pProp, out_values);
}
