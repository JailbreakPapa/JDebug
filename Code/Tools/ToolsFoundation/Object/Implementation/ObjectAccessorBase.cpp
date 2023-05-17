#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Object/ObjectAccessorBase.h>

void wdObjectAccessorBase::StartTransaction(const char* szDisplayString) {}


void wdObjectAccessorBase::CancelTransaction() {}


void wdObjectAccessorBase::FinishTransaction() {}


void wdObjectAccessorBase::BeginTemporaryCommands(const char* szDisplayString, bool bFireEventsWhenUndoingTempCommands /*= false*/) {}


void wdObjectAccessorBase::CancelTemporaryCommands() {}


void wdObjectAccessorBase::FinishTemporaryCommands() {}


wdStatus wdObjectAccessorBase::GetValue(const wdDocumentObject* pObject, const char* szProp, wdVariant& out_value, wdVariant index /*= wdVariant()*/)
{
  const wdAbstractProperty* pProp = pObject->GetType()->FindPropertyByName(szProp);
  if (!pProp)
    return wdStatus(wdFmt("The property '{0}' does not exist in type '{1}'.", szProp, pObject->GetType()->GetTypeName()));
  return GetValue(pObject, pProp, out_value, index);
}


wdStatus wdObjectAccessorBase::SetValue(
  const wdDocumentObject* pObject, const char* szProp, const wdVariant& newValue, wdVariant index /*= wdVariant()*/)
{
  const wdAbstractProperty* pProp = pObject->GetType()->FindPropertyByName(szProp);
  if (!pProp)
    return wdStatus(wdFmt("The property '{0}' does not exist in type '{1}'.", szProp, pObject->GetType()->GetTypeName()));
  return SetValue(pObject, pProp, newValue, index);
}


wdStatus wdObjectAccessorBase::InsertValue(
  const wdDocumentObject* pObject, const char* szProp, const wdVariant& newValue, wdVariant index /*= wdVariant()*/)
{
  const wdAbstractProperty* pProp = pObject->GetType()->FindPropertyByName(szProp);
  if (!pProp)
    return wdStatus(wdFmt("The property '{0}' does not exist in type '{1}'.", szProp, pObject->GetType()->GetTypeName()));
  return InsertValue(pObject, pProp, newValue, index);
}


wdStatus wdObjectAccessorBase::RemoveValue(const wdDocumentObject* pObject, const char* szProp, wdVariant index /*= wdVariant()*/)
{
  const wdAbstractProperty* pProp = pObject->GetType()->FindPropertyByName(szProp);
  if (!pProp)
    return wdStatus(wdFmt("The property '{0}' does not exist in type '{1}'.", szProp, pObject->GetType()->GetTypeName()));
  return RemoveValue(pObject, pProp, index);
}


wdStatus wdObjectAccessorBase::MoveValue(const wdDocumentObject* pObject, const char* szProp, const wdVariant& oldIndex, const wdVariant& newIndex)
{
  const wdAbstractProperty* pProp = pObject->GetType()->FindPropertyByName(szProp);
  if (!pProp)
    return wdStatus(wdFmt("The property '{0}' does not exist in type '{1}'.", szProp, pObject->GetType()->GetTypeName()));
  return MoveValue(pObject, pProp, oldIndex, newIndex);
}


wdStatus wdObjectAccessorBase::GetCount(const wdDocumentObject* pObject, const char* szProp, wdInt32& out_iCount)
{
  const wdAbstractProperty* pProp = pObject->GetType()->FindPropertyByName(szProp);
  if (!pProp)
    return wdStatus(wdFmt("The property '{0}' does not exist in type '{1}'.", szProp, pObject->GetType()->GetTypeName()));
  return GetCount(pObject, pProp, out_iCount);
}


wdStatus wdObjectAccessorBase::AddObject(
  const wdDocumentObject* pParent, const char* szParentProp, const wdVariant& index, const wdRTTI* pType, wdUuid& inout_objectGuid)
{
  const wdAbstractProperty* pProp = pParent->GetType()->FindPropertyByName(szParentProp);
  if (!pProp)
    return wdStatus(wdFmt("The property '{0}' does not exist in type '{1}'.", szParentProp, pParent->GetType()->GetTypeName()));
  return AddObject(pParent, pProp, index, pType, inout_objectGuid);
}

wdStatus wdObjectAccessorBase::MoveObject(
  const wdDocumentObject* pObject, const wdDocumentObject* pNewParent, const char* szParentProp, const wdVariant& index)
{
  const wdAbstractProperty* pProp = pNewParent->GetType()->FindPropertyByName(szParentProp);
  if (!pProp)
    return wdStatus(wdFmt("The property '{0}' does not exist in type '{1}'.", szParentProp, pNewParent->GetType()->GetTypeName()));
  return MoveObject(pObject, pNewParent, pProp, index);
}


wdStatus wdObjectAccessorBase::GetKeys(const wdDocumentObject* pObject, const char* szProp, wdDynamicArray<wdVariant>& out_keys)
{
  const wdAbstractProperty* pProp = pObject->GetType()->FindPropertyByName(szProp);
  if (!pProp)
    return wdStatus(wdFmt("The property '{0}' does not exist in type '{1}'.", szProp, pObject->GetType()->GetTypeName()));
  return GetKeys(pObject, pProp, out_keys);
}


wdStatus wdObjectAccessorBase::GetValues(const wdDocumentObject* pObject, const char* szProp, wdDynamicArray<wdVariant>& out_values)
{
  const wdAbstractProperty* pProp = pObject->GetType()->FindPropertyByName(szProp);
  if (!pProp)
    return wdStatus(wdFmt("The property '{0}' does not exist in type '{1}'.", szProp, pObject->GetType()->GetTypeName()));
  return GetValues(pObject, pProp, out_values);
}

const wdDocumentObject* wdObjectAccessorBase::GetChildObject(const wdDocumentObject* pObject, const char* szProp, wdVariant index)
{
  wdVariant value;
  if (GetValue(pObject, szProp, value, index).Succeeded() && value.IsA<wdUuid>())
  {
    return GetObject(value.Get<wdUuid>());
  }
  return nullptr;
}

wdStatus wdObjectAccessorBase::Clear(const wdDocumentObject* pObject, const char* szProp)
{
  const wdAbstractProperty* pProp = pObject->GetType()->FindPropertyByName(szProp);
  if (!pProp)
    return wdStatus(wdFmt("The property '{0}' does not exist in type '{1}'.", szProp, pObject->GetType()->GetTypeName()));

  wdHybridArray<wdVariant, 8> keys;
  wdStatus res = GetKeys(pObject, pProp, keys);
  if (res.Failed())
    return res;

  for (wdInt32 i = keys.GetCount() - 1; i >= 0; --i)
  {
    res = RemoveValue(pObject, pProp, keys[i]);
    if (res.Failed())
      return res;
  }
  return wdStatus(WD_SUCCESS);
}

wdObjectAccessorBase::wdObjectAccessorBase(const wdDocumentObjectManager* pManager)
  : m_pConstManager(pManager)
{
}

wdObjectAccessorBase::~wdObjectAccessorBase() {}

const wdDocumentObjectManager* wdObjectAccessorBase::GetObjectManager() const
{
  return m_pConstManager;
}

void wdObjectAccessorBase::FireDocumentObjectStructureEvent(const wdDocumentObjectStructureEvent& e)
{
  m_pConstManager->m_StructureEvents.Broadcast(e);
}

void wdObjectAccessorBase::FireDocumentObjectPropertyEvent(const wdDocumentObjectPropertyEvent& e)
{
  m_pConstManager->m_PropertyEvents.Broadcast(e);
}
