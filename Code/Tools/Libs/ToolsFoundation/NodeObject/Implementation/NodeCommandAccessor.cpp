/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Command/NodeCommands.h>
#include <ToolsFoundation/NodeObject/DocumentNodeManager.h>
#include <ToolsFoundation/NodeObject/NodeCommandAccessor.h>

nsNodeCommandAccessor::nsNodeCommandAccessor(nsCommandHistory* pHistory)
  : nsObjectCommandAccessor(pHistory)
{
}

nsNodeCommandAccessor::~nsNodeCommandAccessor() = default;

nsStatus nsNodeCommandAccessor::SetValue(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& newValue, nsVariant index /*= nsVariant()*/)
{
  if (m_pHistory->InTemporaryTransaction() == false && IsDynamicPinProperty(pObject, pProp))
  {
    nsHybridArray<ConnectionInfo, 16> oldConnections;
    NS_SUCCEED_OR_RETURN(DisconnectAllPins(pObject, oldConnections));

    // TODO: remap oldConnections

    NS_SUCCEED_OR_RETURN(nsObjectCommandAccessor::SetValue(pObject, pProp, newValue, index));

    return TryReconnectAllPins(pObject, oldConnections);
  }
  else
  {
    return nsObjectCommandAccessor::SetValue(pObject, pProp, newValue, index);
  }
}

nsStatus nsNodeCommandAccessor::InsertValue(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& newValue, nsVariant index /*= nsVariant()*/)
{
  if (IsDynamicPinProperty(pObject, pProp))
  {
    nsHybridArray<ConnectionInfo, 16> oldConnections;
    NS_SUCCEED_OR_RETURN(DisconnectAllPins(pObject, oldConnections));

    NS_SUCCEED_OR_RETURN(nsObjectCommandAccessor::InsertValue(pObject, pProp, newValue, index));

    return TryReconnectAllPins(pObject, oldConnections);
  }
  else
  {
    return nsObjectCommandAccessor::InsertValue(pObject, pProp, newValue, index);
  }
}

nsStatus nsNodeCommandAccessor::RemoveValue(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant index /*= nsVariant()*/)
{
  if (IsDynamicPinProperty(pObject, pProp))
  {
    nsHybridArray<ConnectionInfo, 16> oldConnections;
    NS_SUCCEED_OR_RETURN(DisconnectAllPins(pObject, oldConnections));

    NS_SUCCEED_OR_RETURN(nsObjectCommandAccessor::RemoveValue(pObject, pProp, index));

    return TryReconnectAllPins(pObject, oldConnections);
  }
  else
  {
    return nsObjectCommandAccessor::RemoveValue(pObject, pProp, index);
  }
}

nsStatus nsNodeCommandAccessor::MoveValue(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& oldIndex, const nsVariant& newIndex)
{
  if (IsDynamicPinProperty(pObject, pProp))
  {
    nsHybridArray<ConnectionInfo, 16> oldConnections;
    NS_SUCCEED_OR_RETURN(DisconnectAllPins(pObject, oldConnections));

    // TODO: remap oldConnections

    NS_SUCCEED_OR_RETURN(nsObjectCommandAccessor::MoveValue(pObject, pProp, oldIndex, newIndex));

    return TryReconnectAllPins(pObject, oldConnections);
  }
  else
  {
    return nsObjectCommandAccessor::MoveValue(pObject, pProp, oldIndex, newIndex);
  }
}

bool nsNodeCommandAccessor::IsDynamicPinProperty(const nsDocumentObject* pObject, const nsAbstractProperty* pProp) const
{
  auto pManager = static_cast<const nsDocumentNodeManager*>(pObject->GetDocumentObjectManager());

  return pManager->IsDynamicPinProperty(pObject, pProp);
}

nsStatus nsNodeCommandAccessor::DisconnectAllPins(const nsDocumentObject* pObject, nsDynamicArray<ConnectionInfo>& out_oldConnections)
{
  auto pManager = static_cast<const nsDocumentNodeManager*>(pObject->GetDocumentObjectManager());

  auto Disconnect = [&](nsArrayPtr<const nsConnection* const> connections) -> nsStatus {
    for (const nsConnection* pConnection : connections)
    {
      auto& connectionInfo = out_oldConnections.ExpandAndGetRef();
      connectionInfo.m_pSource = pConnection->GetSourcePin().GetParent();
      connectionInfo.m_pTarget = pConnection->GetTargetPin().GetParent();
      connectionInfo.m_sSourcePin = pConnection->GetSourcePin().GetName();
      connectionInfo.m_sTargetPin = pConnection->GetTargetPin().GetName();

      NS_SUCCEED_OR_RETURN(nsNodeCommands::DisconnectAndRemoveCommand(m_pHistory, pConnection->GetParent()->GetGuid()));
    }

    return nsStatus(NS_SUCCESS);
  };

  auto inputs = pManager->GetInputPins(pObject);
  for (auto& pInputPin : inputs)
  {
    NS_SUCCEED_OR_RETURN(Disconnect(pManager->GetConnections(*pInputPin)));
  }

  auto outputs = pManager->GetOutputPins(pObject);
  for (auto& pOutputPin : outputs)
  {
    NS_SUCCEED_OR_RETURN(Disconnect(pManager->GetConnections(*pOutputPin)));
  }

  return nsStatus(NS_SUCCESS);
}

nsStatus nsNodeCommandAccessor::TryReconnectAllPins(const nsDocumentObject* pObject, const nsDynamicArray<ConnectionInfo>& oldConnections)
{
  auto pManager = static_cast<const nsDocumentNodeManager*>(pObject->GetDocumentObjectManager());
  const nsRTTI* pConnectionType = pManager->GetConnectionType();

  for (auto& connectionInfo : oldConnections)
  {
    const nsPin* pSourcePin = pManager->GetOutputPinByName(connectionInfo.m_pSource, connectionInfo.m_sSourcePin);
    const nsPin* pTargetPin = pManager->GetInputPinByName(connectionInfo.m_pTarget, connectionInfo.m_sTargetPin);

    // This connection can't be restored because a pin doesn't exist anymore, which is ok in this case.
    if (pSourcePin == nullptr || pTargetPin == nullptr)
      continue;

    // This connection is not valid anymore after pins have changed.
    nsDocumentNodeManager::CanConnectResult res;
    if (pManager->CanConnect(pConnectionType, *pSourcePin, *pTargetPin, res).Failed())
      continue;

    NS_SUCCEED_OR_RETURN(nsNodeCommands::AddAndConnectCommand(m_pHistory, pConnectionType, *pSourcePin, *pTargetPin));
  }

  return nsStatus(NS_SUCCESS);
}
