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
  if (m_pHistory->InTemporaryTransaction() == false)
  {
    auto pNodeObject = pObject;
    auto pDynamicPinProperty = pProp;

    if (IsNode(pObject) == false)
    {
      auto pParent = pObject->GetParent();
      if (pParent != nullptr && IsNode(pParent))
      {
        pNodeObject = pParent;
        pDynamicPinProperty = pParent->GetType()->FindPropertyByName(pObject->GetParentProperty());
      }
    }

    if (IsDynamicPinProperty(pNodeObject, pDynamicPinProperty))
    {
      nsHybridArray<ConnectionInfo, 16> oldConnections;
      NS_SUCCEED_OR_RETURN(DisconnectAllPins(pNodeObject, oldConnections));

      // TODO: remap oldConnections

      NS_SUCCEED_OR_RETURN(nsObjectCommandAccessor::SetValue(pObject, pProp, newValue, index));

      return TryReconnectAllPins(pNodeObject, oldConnections);
    }
  }

  return nsObjectCommandAccessor::SetValue(pObject, pProp, newValue, index);
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

nsStatus nsNodeCommandAccessor::AddObject(const nsDocumentObject* pParent, const nsAbstractProperty* pParentProp, const nsVariant& index, const nsRTTI* pType, nsUuid& inout_objectGuid)
{
  if (IsDynamicPinProperty(pParent, pParentProp))
  {
    nsHybridArray<ConnectionInfo, 16> oldConnections;
    NS_SUCCEED_OR_RETURN(DisconnectAllPins(pParent, oldConnections));

    // TODO: remap oldConnections

    NS_SUCCEED_OR_RETURN(nsObjectCommandAccessor::AddObject(pParent, pParentProp, index, pType, inout_objectGuid));

    return TryReconnectAllPins(pParent, oldConnections);
  }
  else
  {
    return nsObjectCommandAccessor::AddObject(pParent, pParentProp, index, pType, inout_objectGuid);
  }
}

nsStatus nsNodeCommandAccessor::RemoveObject(const nsDocumentObject* pObject)
{
  if (const nsDocumentObject* pParent = pObject->GetParent())
  {
    const nsAbstractProperty* pProp = pParent->GetType()->FindPropertyByName(pObject->GetParentProperty());
    if (IsDynamicPinProperty(pParent, pProp))
    {
      nsHybridArray<ConnectionInfo, 16> oldConnections;
      NS_SUCCEED_OR_RETURN(DisconnectAllPins(pParent, oldConnections));

      // TODO: remap oldConnections

      NS_SUCCEED_OR_RETURN(nsObjectCommandAccessor::RemoveObject(pObject));

      return TryReconnectAllPins(pParent, oldConnections);
    }
  }

  return nsObjectCommandAccessor::RemoveObject(pObject);
}


bool nsNodeCommandAccessor::IsNode(const nsDocumentObject* pObject) const
{
  auto pManager = static_cast<const nsDocumentNodeManager*>(pObject->GetDocumentObjectManager());

  return pManager->IsNode(pObject);
}

bool nsNodeCommandAccessor::IsDynamicPinProperty(const nsDocumentObject* pObject, const nsAbstractProperty* pProp) const
{
  auto pManager = static_cast<const nsDocumentNodeManager*>(pObject->GetDocumentObjectManager());

  return pManager->IsDynamicPinProperty(pObject, pProp);
}

nsStatus nsNodeCommandAccessor::DisconnectAllPins(const nsDocumentObject* pObject, nsDynamicArray<ConnectionInfo>& out_oldConnections)
{
  auto pManager = static_cast<const nsDocumentNodeManager*>(pObject->GetDocumentObjectManager());

  auto Disconnect = [&](nsArrayPtr<const nsConnection* const> connections) -> nsStatus
  {
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
