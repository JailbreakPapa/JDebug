#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Command/TreeCommands.h>
#include <ToolsFoundation/CommandHistory/CommandHistory.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/Object/ObjectCommandAccessor.h>

nsObjectCommandAccessor::nsObjectCommandAccessor(nsCommandHistory* pHistory)
  : nsObjectDirectAccessor(const_cast<nsDocumentObjectManager*>(pHistory->GetDocument()->GetObjectManager()))
  , m_pHistory(pHistory)
{
}

void nsObjectCommandAccessor::StartTransaction(nsStringView sDisplayString)
{
  m_pHistory->StartTransaction(sDisplayString);
}

void nsObjectCommandAccessor::CancelTransaction()
{
  m_pHistory->CancelTransaction();
}

void nsObjectCommandAccessor::FinishTransaction()
{
  m_pHistory->FinishTransaction();
}

void nsObjectCommandAccessor::BeginTemporaryCommands(nsStringView sDisplayString, bool bFireEventsWhenUndoingTempCommands /*= false*/)
{
  m_pHistory->BeginTemporaryCommands(sDisplayString, bFireEventsWhenUndoingTempCommands);
}

void nsObjectCommandAccessor::CancelTemporaryCommands()
{
  m_pHistory->CancelTemporaryCommands();
}

void nsObjectCommandAccessor::FinishTemporaryCommands()
{
  m_pHistory->FinishTemporaryCommands();
}

nsStatus nsObjectCommandAccessor::SetValue(
  const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& newValue, nsVariant index /*= nsVariant()*/)
{
  nsSetObjectPropertyCommand cmd;
  cmd.m_Object = pObject->GetGuid();
  cmd.m_NewValue = newValue;
  cmd.m_Index = index;
  cmd.m_sProperty = pProp->GetPropertyName();
  return m_pHistory->AddCommand(cmd);
}

nsStatus nsObjectCommandAccessor::InsertValue(
  const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& newValue, nsVariant index /*= nsVariant()*/)
{
  nsInsertObjectPropertyCommand cmd;
  cmd.m_Object = pObject->GetGuid();
  cmd.m_NewValue = newValue;
  cmd.m_Index = index;
  cmd.m_sProperty = pProp->GetPropertyName();
  return m_pHistory->AddCommand(cmd);
}

nsStatus nsObjectCommandAccessor::RemoveValue(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant index /*= nsVariant()*/)
{
  nsRemoveObjectPropertyCommand cmd;
  cmd.m_Object = pObject->GetGuid();
  cmd.m_Index = index;
  cmd.m_sProperty = pProp->GetPropertyName();
  return m_pHistory->AddCommand(cmd);
}

nsStatus nsObjectCommandAccessor::MoveValue(
  const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& oldIndex, const nsVariant& newIndex)
{
  nsMoveObjectPropertyCommand cmd;
  cmd.m_Object = pObject->GetGuid();
  cmd.m_OldIndex = oldIndex;
  cmd.m_NewIndex = newIndex;
  cmd.m_sProperty = pProp->GetPropertyName();
  return m_pHistory->AddCommand(cmd);
}

nsStatus nsObjectCommandAccessor::AddObject(
  const nsDocumentObject* pParent, const nsAbstractProperty* pParentProp, const nsVariant& index, const nsRTTI* pType, nsUuid& inout_objectGuid)
{
  nsAddObjectCommand cmd;
  cmd.m_Parent = pParent ? pParent->GetGuid() : nsUuid();
  cmd.m_Index = index;
  cmd.m_pType = pType;
  cmd.m_NewObjectGuid = inout_objectGuid;
  cmd.m_sParentProperty = pParentProp ? pParentProp->GetPropertyName() : "Children";
  nsStatus res = m_pHistory->AddCommand(cmd);
  if (res.m_Result.Succeeded())
    inout_objectGuid = cmd.m_NewObjectGuid;
  return res;
}

nsStatus nsObjectCommandAccessor::RemoveObject(const nsDocumentObject* pObject)
{
  nsRemoveObjectCommand cmd;
  cmd.m_Object = pObject->GetGuid();
  return m_pHistory->AddCommand(cmd);
}

nsStatus nsObjectCommandAccessor::MoveObject(
  const nsDocumentObject* pObject, const nsDocumentObject* pNewParent, const nsAbstractProperty* pParentProp, const nsVariant& index)
{
  nsMoveObjectCommand cmd;
  cmd.m_NewParent = pNewParent ? pNewParent->GetGuid() : nsUuid();
  cmd.m_Object = pObject->GetGuid();
  cmd.m_Index = index;
  cmd.m_sParentProperty = pParentProp->GetPropertyName();
  return m_pHistory->AddCommand(cmd);
}
