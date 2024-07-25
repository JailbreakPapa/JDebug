#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/Object/ObjectDirectAccessor.h>

nsObjectDirectAccessor::nsObjectDirectAccessor(nsDocumentObjectManager* pManager)
  : nsObjectAccessorBase(pManager)
  , m_pManager(pManager)
{
}

const nsDocumentObject* nsObjectDirectAccessor::GetObject(const nsUuid& object)
{
  return m_pManager->GetObject(object);
}

nsStatus nsObjectDirectAccessor::GetValue(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant& out_value, nsVariant index)
{
  if (pProp == nullptr)
    return nsStatus("Property is null.");

  nsStatus res;
  out_value = pObject->GetTypeAccessor().GetValue(pProp->GetPropertyName(), index, &res);
  return res;
}

nsStatus nsObjectDirectAccessor::SetValue(
  const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& newValue, nsVariant index)
{
  nsDocumentObject* pObj = m_pManager->GetObject(pObject->GetGuid());
  NS_ASSERT_DEBUG(pObj, "Object is not part of this document manager.");
  bool bRes = pObj->GetTypeAccessor().SetValue(pProp->GetPropertyName(), newValue, index);
  return nsStatus(bRes ? NS_SUCCESS : NS_FAILURE);
}

nsStatus nsObjectDirectAccessor::InsertValue(
  const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& newValue, nsVariant index)
{
  nsDocumentObject* pObj = m_pManager->GetObject(pObject->GetGuid());
  NS_ASSERT_DEBUG(pObj, "Object is not part of this document manager.");
  bool bRes = pObj->GetTypeAccessor().InsertValue(pProp->GetPropertyName(), index, newValue);
  return nsStatus(bRes ? NS_SUCCESS : NS_FAILURE);
}

nsStatus nsObjectDirectAccessor::RemoveValue(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant index)
{
  nsDocumentObject* pObj = m_pManager->GetObject(pObject->GetGuid());
  NS_ASSERT_DEBUG(pObj, "Object is not part of this document manager.");
  bool bRes = pObj->GetTypeAccessor().RemoveValue(pProp->GetPropertyName(), index);
  return nsStatus(bRes ? NS_SUCCESS : NS_FAILURE);
}

nsStatus nsObjectDirectAccessor::MoveValue(
  const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& oldIndex, const nsVariant& newIndex)
{
  nsDocumentObject* pObj = m_pManager->GetObject(pObject->GetGuid());
  NS_ASSERT_DEBUG(pObj, "Object is not part of this document manager.");
  bool bRes = pObj->GetTypeAccessor().MoveValue(pProp->GetPropertyName(), oldIndex, newIndex);
  return nsStatus(bRes ? NS_SUCCESS : NS_FAILURE);
}

nsStatus nsObjectDirectAccessor::GetCount(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsInt32& out_iCount)
{
  out_iCount = pObject->GetTypeAccessor().GetCount(pProp->GetPropertyName());
  return nsStatus(NS_SUCCESS);
}

nsStatus nsObjectDirectAccessor::AddObject(
  const nsDocumentObject* pParent, const nsAbstractProperty* pParentProp, const nsVariant& index, const nsRTTI* pType, nsUuid& inout_objectGuid)
{
  NS_SUCCEED_OR_RETURN(m_pManager->CanAdd(pType, pParent, pParentProp->GetPropertyName(), index));

  nsDocumentObject* pPar = m_pManager->GetObject(pParent->GetGuid());
  NS_ASSERT_DEBUG(pPar, "Parent is not part of this document manager.");

  if (!inout_objectGuid.IsValid())
    inout_objectGuid = nsUuid::MakeUuid();
  nsDocumentObject* pObj = m_pManager->CreateObject(pType, inout_objectGuid);
  m_pManager->AddObject(pObj, pPar, pParentProp->GetPropertyName(), index);
  return nsStatus(NS_SUCCESS);
}

nsStatus nsObjectDirectAccessor::RemoveObject(const nsDocumentObject* pObject)
{
  NS_SUCCEED_OR_RETURN(m_pManager->CanRemove(pObject));

  nsDocumentObject* pObj = m_pManager->GetObject(pObject->GetGuid());
  NS_ASSERT_DEBUG(pObj, "Object is not part of this document manager.");
  m_pManager->RemoveObject(pObj);
  return nsStatus(NS_SUCCESS);
}

nsStatus nsObjectDirectAccessor::MoveObject(
  const nsDocumentObject* pObject, const nsDocumentObject* pNewParent, const nsAbstractProperty* pParentProp, const nsVariant& index)
{
  NS_SUCCEED_OR_RETURN(m_pManager->CanMove(pObject, pNewParent, pParentProp->GetPropertyName(), index));

  nsDocumentObject* pObj = m_pManager->GetObject(pObject->GetGuid());
  NS_ASSERT_DEBUG(pObj, "Object is not part of this document manager.");
  nsDocumentObject* pPar = m_pManager->GetObject(pNewParent->GetGuid());
  NS_ASSERT_DEBUG(pPar, "Parent is not part of this document manager.");

  m_pManager->MoveObject(pObj, pPar, pParentProp->GetPropertyName(), index);
  return nsStatus(NS_SUCCESS);
}

nsStatus nsObjectDirectAccessor::GetKeys(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsDynamicArray<nsVariant>& out_keys)
{
  bool bRes = pObject->GetTypeAccessor().GetKeys(pProp->GetPropertyName(), out_keys);
  return nsStatus(bRes ? NS_SUCCESS : NS_FAILURE);
}

nsStatus nsObjectDirectAccessor::GetValues(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsDynamicArray<nsVariant>& out_values)
{
  bool bRes = pObject->GetTypeAccessor().GetValues(pProp->GetPropertyName(), out_values);
  return nsStatus(bRes ? NS_SUCCESS : NS_FAILURE);
}
