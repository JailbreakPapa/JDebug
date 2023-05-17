#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/Object/ObjectDirectAccessor.h>

wdObjectDirectAccessor::wdObjectDirectAccessor(wdDocumentObjectManager* pManager)
  : wdObjectAccessorBase(pManager)
  , m_pManager(pManager)
{
}

const wdDocumentObject* wdObjectDirectAccessor::GetObject(const wdUuid& object)
{
  return m_pManager->GetObject(object);
}

wdStatus wdObjectDirectAccessor::GetValue(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant& out_value, wdVariant index)
{
  if (pProp == nullptr)
    return wdStatus("Property is null.");

  wdStatus res;
  out_value = pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName(), index, &res);
  return res;
}

wdStatus wdObjectDirectAccessor::SetValue(
  const wdDocumentObject* pObject, const wdAbstractProperty* pProp, const wdVariant& newValue, wdVariant index)
{
  wdDocumentObject* pObj = m_pManager->GetObject(pObject->GetGuid());
  WD_ASSERT_DEBUG(pObj, "Object is not part of this document manager.");
  bool bRes = pObj->GetTypeAccessor().SetValue(pProp->GetPropertyName(), newValue, index);
  return wdStatus(bRes ? WD_SUCCESS : WD_FAILURE);
}

wdStatus wdObjectDirectAccessor::InsertValue(
  const wdDocumentObject* pObject, const wdAbstractProperty* pProp, const wdVariant& newValue, wdVariant index)
{
  wdDocumentObject* pObj = m_pManager->GetObject(pObject->GetGuid());
  WD_ASSERT_DEBUG(pObj, "Object is not part of this document manager.");
  bool bRes = pObj->GetTypeAccessor().InsertValue(pProp->GetPropertyName(), index, newValue);
  return wdStatus(bRes ? WD_SUCCESS : WD_FAILURE);
}

wdStatus wdObjectDirectAccessor::RemoveValue(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant index)
{
  wdDocumentObject* pObj = m_pManager->GetObject(pObject->GetGuid());
  WD_ASSERT_DEBUG(pObj, "Object is not part of this document manager.");
  bool bRes = pObj->GetTypeAccessor().RemoveValue(pProp->GetPropertyName(), index);
  return wdStatus(bRes ? WD_SUCCESS : WD_FAILURE);
}

wdStatus wdObjectDirectAccessor::MoveValue(
  const wdDocumentObject* pObject, const wdAbstractProperty* pProp, const wdVariant& oldIndex, const wdVariant& newIndex)
{
  wdDocumentObject* pObj = m_pManager->GetObject(pObject->GetGuid());
  WD_ASSERT_DEBUG(pObj, "Object is not part of this document manager.");
  bool bRes = pObj->GetTypeAccessor().MoveValue(pProp->GetPropertyName(), oldIndex, newIndex);
  return wdStatus(bRes ? WD_SUCCESS : WD_FAILURE);
}

wdStatus wdObjectDirectAccessor::GetCount(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdInt32& out_iCount)
{
  out_iCount = pObject->GetTypeAccessor().GetCount(pProp->GetPropertyName());
  return wdStatus(WD_SUCCESS);
}

wdStatus wdObjectDirectAccessor::AddObject(
  const wdDocumentObject* pParent, const wdAbstractProperty* pParentProp, const wdVariant& index, const wdRTTI* pType, wdUuid& inout_objectGuid)
{
  WD_SUCCEED_OR_RETURN(m_pManager->CanAdd(pType, pParent, pParentProp->GetPropertyName(), index));

  wdDocumentObject* pPar = m_pManager->GetObject(pParent->GetGuid());
  WD_ASSERT_DEBUG(pPar, "Parent is not part of this document manager.");

  if (!inout_objectGuid.IsValid())
    inout_objectGuid.CreateNewUuid();
  wdDocumentObject* pObj = m_pManager->CreateObject(pType, inout_objectGuid);
  m_pManager->AddObject(pObj, pPar, pParentProp->GetPropertyName(), index);
  return wdStatus(WD_SUCCESS);
}

wdStatus wdObjectDirectAccessor::RemoveObject(const wdDocumentObject* pObject)
{
  WD_SUCCEED_OR_RETURN(m_pManager->CanRemove(pObject));

  wdDocumentObject* pObj = m_pManager->GetObject(pObject->GetGuid());
  WD_ASSERT_DEBUG(pObj, "Object is not part of this document manager.");
  m_pManager->RemoveObject(pObj);
  return wdStatus(WD_SUCCESS);
}

wdStatus wdObjectDirectAccessor::MoveObject(
  const wdDocumentObject* pObject, const wdDocumentObject* pNewParent, const wdAbstractProperty* pParentProp, const wdVariant& index)
{
  WD_SUCCEED_OR_RETURN(m_pManager->CanMove(pObject, pNewParent, pParentProp->GetPropertyName(), index));

  wdDocumentObject* pObj = m_pManager->GetObject(pObject->GetGuid());
  WD_ASSERT_DEBUG(pObj, "Object is not part of this document manager.");
  wdDocumentObject* pPar = m_pManager->GetObject(pNewParent->GetGuid());
  WD_ASSERT_DEBUG(pPar, "Parent is not part of this document manager.");

  m_pManager->MoveObject(pObj, pPar, pParentProp->GetPropertyName(), index);
  return wdStatus(WD_SUCCESS);
}

wdStatus wdObjectDirectAccessor::GetKeys(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdDynamicArray<wdVariant>& out_keys)
{
  bool bRes = pObject->GetTypeAccessor().GetKeys(pProp->GetPropertyName(), out_keys);
  return wdStatus(bRes ? WD_SUCCESS : WD_FAILURE);
}

wdStatus wdObjectDirectAccessor::GetValues(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdDynamicArray<wdVariant>& out_values)
{
  bool bRes = pObject->GetTypeAccessor().GetValues(pProp->GetPropertyName(), out_values);
  return wdStatus(bRes ? WD_SUCCESS : WD_FAILURE);
}
