#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Command/TreeCommands.h>
#include <ToolsFoundation/CommandHistory/CommandHistory.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/Object/ObjectCommandAccessor.h>

wdObjectCommandAccessor::wdObjectCommandAccessor(wdCommandHistory* pHistory)
  : wdObjectDirectAccessor(const_cast<wdDocumentObjectManager*>(pHistory->GetDocument()->GetObjectManager()))
  , m_pHistory(pHistory)
{
}

void wdObjectCommandAccessor::StartTransaction(const char* szDisplayString)
{
  m_pHistory->StartTransaction(szDisplayString);
}

void wdObjectCommandAccessor::CancelTransaction()
{
  m_pHistory->CancelTransaction();
}

void wdObjectCommandAccessor::FinishTransaction()
{
  m_pHistory->FinishTransaction();
}

void wdObjectCommandAccessor::BeginTemporaryCommands(const char* szDisplayString, bool bFireEventsWhenUndoingTempCommands /*= false*/)
{
  m_pHistory->BeginTemporaryCommands(szDisplayString, bFireEventsWhenUndoingTempCommands);
}

void wdObjectCommandAccessor::CancelTemporaryCommands()
{
  m_pHistory->CancelTemporaryCommands();
}

void wdObjectCommandAccessor::FinishTemporaryCommands()
{
  m_pHistory->FinishTemporaryCommands();
}

wdStatus wdObjectCommandAccessor::SetValue(
  const wdDocumentObject* pObject, const wdAbstractProperty* pProp, const wdVariant& newValue, wdVariant index /*= wdVariant()*/)
{
  wdSetObjectPropertyCommand cmd;
  cmd.m_Object = pObject->GetGuid();
  cmd.m_NewValue = newValue;
  cmd.m_Index = index;
  cmd.m_sProperty = pProp->GetPropertyName();
  return m_pHistory->AddCommand(cmd);
}

wdStatus wdObjectCommandAccessor::InsertValue(
  const wdDocumentObject* pObject, const wdAbstractProperty* pProp, const wdVariant& newValue, wdVariant index /*= wdVariant()*/)
{
  wdInsertObjectPropertyCommand cmd;
  cmd.m_Object = pObject->GetGuid();
  cmd.m_NewValue = newValue;
  cmd.m_Index = index;
  cmd.m_sProperty = pProp->GetPropertyName();
  return m_pHistory->AddCommand(cmd);
}

wdStatus wdObjectCommandAccessor::RemoveValue(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant index /*= wdVariant()*/)
{
  wdRemoveObjectPropertyCommand cmd;
  cmd.m_Object = pObject->GetGuid();
  cmd.m_Index = index;
  cmd.m_sProperty = pProp->GetPropertyName();
  return m_pHistory->AddCommand(cmd);
}

wdStatus wdObjectCommandAccessor::MoveValue(
  const wdDocumentObject* pObject, const wdAbstractProperty* pProp, const wdVariant& oldIndex, const wdVariant& newIndex)
{
  wdMoveObjectPropertyCommand cmd;
  cmd.m_Object = pObject->GetGuid();
  cmd.m_OldIndex = oldIndex;
  cmd.m_NewIndex = newIndex;
  cmd.m_sProperty = pProp->GetPropertyName();
  return m_pHistory->AddCommand(cmd);
}

wdStatus wdObjectCommandAccessor::AddObject(
  const wdDocumentObject* pParent, const wdAbstractProperty* pParentProp, const wdVariant& index, const wdRTTI* pType, wdUuid& inout_objectGuid)
{
  wdAddObjectCommand cmd;
  cmd.m_Parent = pParent ? pParent->GetGuid() : wdUuid();
  cmd.m_Index = index;
  cmd.m_pType = pType;
  cmd.m_NewObjectGuid = inout_objectGuid;
  cmd.m_sParentProperty = pParentProp ? pParentProp->GetPropertyName() : "Children";
  wdStatus res = m_pHistory->AddCommand(cmd);
  if (res.m_Result.Succeeded())
    inout_objectGuid = cmd.m_NewObjectGuid;
  return res;
}

wdStatus wdObjectCommandAccessor::RemoveObject(const wdDocumentObject* pObject)
{
  wdRemoveObjectCommand cmd;
  cmd.m_Object = pObject->GetGuid();
  return m_pHistory->AddCommand(cmd);
}

wdStatus wdObjectCommandAccessor::MoveObject(
  const wdDocumentObject* pObject, const wdDocumentObject* pNewParent, const wdAbstractProperty* pParentProp, const wdVariant& index)
{
  wdMoveObjectCommand cmd;
  cmd.m_NewParent = pNewParent ? pNewParent->GetGuid() : wdUuid();
  cmd.m_Object = pObject->GetGuid();
  cmd.m_Index = index;
  cmd.m_sParentProperty = pParentProp->GetPropertyName();
  return m_pHistory->AddCommand(cmd);
}
