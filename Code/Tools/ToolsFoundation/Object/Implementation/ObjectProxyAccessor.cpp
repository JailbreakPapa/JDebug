#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Object/ObjectProxyAccessor.h>

wdObjectProxyAccessor::wdObjectProxyAccessor(wdObjectAccessorBase* pSource)
  : wdObjectAccessorBase(pSource->GetObjectManager())
  , m_pSource(pSource)
{
}

wdObjectProxyAccessor::~wdObjectProxyAccessor() {}

void wdObjectProxyAccessor::StartTransaction(const char* szDisplayString)
{
  m_pSource->StartTransaction(szDisplayString);
}

void wdObjectProxyAccessor::CancelTransaction()
{
  m_pSource->CancelTransaction();
}

void wdObjectProxyAccessor::FinishTransaction()
{
  m_pSource->FinishTransaction();
}

void wdObjectProxyAccessor::BeginTemporaryCommands(const char* szDisplayString, bool bFireEventsWhenUndoingTempCommands /*= false*/)
{
  m_pSource->BeginTemporaryCommands(szDisplayString, bFireEventsWhenUndoingTempCommands);
}

void wdObjectProxyAccessor::CancelTemporaryCommands()
{
  m_pSource->CancelTemporaryCommands();
}

void wdObjectProxyAccessor::FinishTemporaryCommands()
{
  m_pSource->FinishTemporaryCommands();
}

const wdDocumentObject* wdObjectProxyAccessor::GetObject(const wdUuid& object)
{
  return m_pSource->GetObject(object);
}

wdStatus wdObjectProxyAccessor::GetValue(
  const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant& out_value, wdVariant index /*= wdVariant()*/)
{
  return m_pSource->GetValue(pObject, pProp, out_value, index);
}

wdStatus wdObjectProxyAccessor::SetValue(
  const wdDocumentObject* pObject, const wdAbstractProperty* pProp, const wdVariant& newValue, wdVariant index /*= wdVariant()*/)
{
  return m_pSource->SetValue(pObject, pProp, newValue, index);
}

wdStatus wdObjectProxyAccessor::InsertValue(
  const wdDocumentObject* pObject, const wdAbstractProperty* pProp, const wdVariant& newValue, wdVariant index /*= wdVariant()*/)
{
  return m_pSource->InsertValue(pObject, pProp, newValue, index);
}

wdStatus wdObjectProxyAccessor::RemoveValue(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant index /*= wdVariant()*/)
{
  return m_pSource->RemoveValue(pObject, pProp, index);
}

wdStatus wdObjectProxyAccessor::MoveValue(
  const wdDocumentObject* pObject, const wdAbstractProperty* pProp, const wdVariant& oldIndex, const wdVariant& newIndex)
{
  return m_pSource->MoveValue(pObject, pProp, oldIndex, newIndex);
}

wdStatus wdObjectProxyAccessor::GetCount(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdInt32& out_iCount)
{
  return m_pSource->GetCount(pObject, pProp, out_iCount);
}

wdStatus wdObjectProxyAccessor::AddObject(
  const wdDocumentObject* pParent, const wdAbstractProperty* pParentProp, const wdVariant& index, const wdRTTI* pType, wdUuid& inout_objectGuid)
{
  return m_pSource->AddObject(pParent, pParentProp, index, pType, inout_objectGuid);
}

wdStatus wdObjectProxyAccessor::RemoveObject(const wdDocumentObject* pObject)
{
  return m_pSource->RemoveObject(pObject);
}

wdStatus wdObjectProxyAccessor::MoveObject(
  const wdDocumentObject* pObject, const wdDocumentObject* pNewParent, const wdAbstractProperty* pParentProp, const wdVariant& index)
{
  return m_pSource->MoveObject(pObject, pNewParent, pParentProp, index);
}

wdStatus wdObjectProxyAccessor::GetKeys(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdDynamicArray<wdVariant>& out_keys)
{
  return m_pSource->GetKeys(pObject, pProp, out_keys);
}

wdStatus wdObjectProxyAccessor::GetValues(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdDynamicArray<wdVariant>& out_values)
{
  return m_pSource->GetValues(pObject, pProp, out_values);
}
