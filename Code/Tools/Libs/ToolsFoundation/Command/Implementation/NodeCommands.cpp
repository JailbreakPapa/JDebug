/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Command/NodeCommands.h>
#include <ToolsFoundation/Command/TreeCommands.h>
#include <ToolsFoundation/NodeObject/DocumentNodeManager.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsRemoveNodeCommand, 1, nsRTTIDefaultAllocator<nsRemoveNodeCommand>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ObjectGuid", m_Object),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMoveNodeCommand, 1, nsRTTIDefaultAllocator<nsMoveNodeCommand>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ObjectGuid", m_Object),
    NS_MEMBER_PROPERTY("NewPos", m_NewPos),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsConnectNodePinsCommand, 1, nsRTTIDefaultAllocator<nsConnectNodePinsCommand>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ConnectionGuid", m_ConnectionObject),
    NS_MEMBER_PROPERTY("SourceGuid", m_ObjectSource),
    NS_MEMBER_PROPERTY("TargetGuid", m_ObjectTarget),
    NS_MEMBER_PROPERTY("SourcePin", m_sSourcePin),
    NS_MEMBER_PROPERTY("TargetPin", m_sTargetPin),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDisconnectNodePinsCommand, 1, nsRTTIDefaultAllocator<nsDisconnectNodePinsCommand>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ConnectionGuid", m_ConnectionObject),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

////////////////////////////////////////////////////////////////////////
// nsRemoveNodeCommand
////////////////////////////////////////////////////////////////////////

nsRemoveNodeCommand::nsRemoveNodeCommand() = default;

nsStatus nsRemoveNodeCommand::DoInternal(bool bRedo)
{
  nsDocument* pDocument = GetDocument();
  nsDocumentNodeManager* pManager = static_cast<nsDocumentNodeManager*>(pDocument->GetObjectManager());

  auto RemoveConnections = [&](const nsPin& pin) {
    while (true)
    {
      auto connections = pManager->GetConnections(pin);

      if (connections.IsEmpty())
        break;

      nsDisconnectNodePinsCommand cmd;
      cmd.m_ConnectionObject = connections[0]->GetParent()->GetGuid();
      nsStatus res = AddSubCommand(cmd);
      if (res.m_Result.Succeeded())
      {
        nsRemoveObjectCommand remove;
        remove.m_Object = cmd.m_ConnectionObject;
        res = AddSubCommand(remove);
      }

      NS_SUCCEED_OR_RETURN(res);
    }
    return nsStatus(NS_SUCCESS);
  };

  if (!bRedo)
  {
    m_pObject = pManager->GetObject(m_Object);
    if (m_pObject == nullptr)
      return nsStatus("Remove Node: The given object does not exist!");

    auto inputs = pManager->GetInputPins(m_pObject);
    for (auto& pPinTarget : inputs)
    {
      NS_SUCCEED_OR_RETURN(RemoveConnections(*pPinTarget));
    }

    auto outputs = pManager->GetOutputPins(m_pObject);
    for (auto& pPinSource : outputs)
    {
      NS_SUCCEED_OR_RETURN(RemoveConnections(*pPinSource));
    }

    nsRemoveObjectCommand cmd;
    cmd.m_Object = m_Object;
    auto res = AddSubCommand(cmd);
    if (res.m_Result.Failed())
    {
      return res;
    }
  }
  return nsStatus(NS_SUCCESS);
}

nsStatus nsRemoveNodeCommand::UndoInternal(bool bFireEvents)
{
  NS_ASSERT_DEV(bFireEvents, "This command does not support temporary commands");
  return nsStatus(NS_SUCCESS);
}

void nsRemoveNodeCommand::CleanupInternal(CommandState state) {}


////////////////////////////////////////////////////////////////////////
// nsMoveObjectCommand
////////////////////////////////////////////////////////////////////////

nsMoveNodeCommand::nsMoveNodeCommand() = default;

nsStatus nsMoveNodeCommand::DoInternal(bool bRedo)
{
  nsDocument* pDocument = GetDocument();
  nsDocumentNodeManager* pManager = static_cast<nsDocumentNodeManager*>(pDocument->GetObjectManager());

  if (!bRedo)
  {
    m_pObject = pDocument->GetObjectManager()->GetObject(m_Object);
    if (m_pObject == nullptr)
      return nsStatus("Move Node: The given object does not exist!");

    m_vOldPos = pManager->GetNodePos(m_pObject);
    NS_SUCCEED_OR_RETURN(pManager->CanMoveNode(m_pObject, m_NewPos));
  }

  pManager->MoveNode(m_pObject, m_NewPos);
  return nsStatus(NS_SUCCESS);
}

nsStatus nsMoveNodeCommand::UndoInternal(bool bFireEvents)
{
  nsDocument* pDocument = GetDocument();
  nsDocumentNodeManager* pManager = static_cast<nsDocumentNodeManager*>(pDocument->GetObjectManager());
  NS_ASSERT_DEV(bFireEvents, "This command does not support temporary commands");

  NS_SUCCEED_OR_RETURN(pManager->CanMoveNode(m_pObject, m_vOldPos));

  pManager->MoveNode(m_pObject, m_vOldPos);

  return nsStatus(NS_SUCCESS);
}


////////////////////////////////////////////////////////////////////////
// nsConnectNodePinsCommand
////////////////////////////////////////////////////////////////////////

nsConnectNodePinsCommand::nsConnectNodePinsCommand() = default;

nsStatus nsConnectNodePinsCommand::DoInternal(bool bRedo)
{
  nsDocument* pDocument = GetDocument();
  nsDocumentNodeManager* pManager = static_cast<nsDocumentNodeManager*>(pDocument->GetObjectManager());

  if (!bRedo)
  {
    m_pConnectionObject = pManager->GetObject(m_ConnectionObject);
    if (!pManager->IsConnection(m_pConnectionObject))
      return nsStatus("Connect Node Pins: The given connection object is not valid connection!");

    m_pObjectSource = pManager->GetObject(m_ObjectSource);
    if (m_pObjectSource == nullptr)
      return nsStatus("Connect Node Pins: The given node does not exist!");
    m_pObjectTarget = pManager->GetObject(m_ObjectTarget);
    if (m_pObjectTarget == nullptr)
      return nsStatus("Connect Node Pins: The given node does not exist!");
  }

  const nsPin* pOutput = pManager->GetOutputPinByName(m_pObjectSource, m_sSourcePin);
  if (pOutput == nullptr)
    return nsStatus("Connect Node Pins: The given pin does not exist!");

  const nsPin* pInput = pManager->GetInputPinByName(m_pObjectTarget, m_sTargetPin);
  if (pInput == nullptr)
    return nsStatus("Connect Node Pins: The given pin does not exist!");

  nsDocumentNodeManager::CanConnectResult res;
  NS_SUCCEED_OR_RETURN(pManager->CanConnect(m_pConnectionObject->GetType(), *pOutput, *pInput, res));

  pManager->Connect(m_pConnectionObject, *pOutput, *pInput);
  return nsStatus(NS_SUCCESS);
}

nsStatus nsConnectNodePinsCommand::UndoInternal(bool bFireEvents)
{
  nsDocument* pDocument = GetDocument();
  nsDocumentNodeManager* pManager = static_cast<nsDocumentNodeManager*>(pDocument->GetObjectManager());

  NS_SUCCEED_OR_RETURN(pManager->CanDisconnect(m_pConnectionObject));

  pManager->Disconnect(m_pConnectionObject);
  return nsStatus(NS_SUCCESS);
}


////////////////////////////////////////////////////////////////////////
// nsDisconnectNodePinsCommand
////////////////////////////////////////////////////////////////////////

nsDisconnectNodePinsCommand::nsDisconnectNodePinsCommand() = default;

nsStatus nsDisconnectNodePinsCommand::DoInternal(bool bRedo)
{
  nsDocument* pDocument = GetDocument();
  nsDocumentNodeManager* pManager = static_cast<nsDocumentNodeManager*>(pDocument->GetObjectManager());

  if (!bRedo)
  {
    m_pConnectionObject = pManager->GetObject(m_ConnectionObject);
    if (!pManager->IsConnection(m_pConnectionObject))
      return nsStatus("Disconnect Node Pins: The given connection object is not valid connection!");

    NS_SUCCEED_OR_RETURN(pManager->CanRemove(m_pConnectionObject));

    const nsConnection& connection = pManager->GetConnection(m_pConnectionObject);
    const nsPin& pinSource = connection.GetSourcePin();
    const nsPin& pinTarget = connection.GetTargetPin();

    m_pObjectSource = pinSource.GetParent();
    m_pObjectTarget = pinTarget.GetParent();
    m_sSourcePin = pinSource.GetName();
    m_sTargetPin = pinTarget.GetName();
  }

  NS_SUCCEED_OR_RETURN(pManager->CanDisconnect(m_pConnectionObject));

  pManager->Disconnect(m_pConnectionObject);

  return nsStatus(NS_SUCCESS);
}

nsStatus nsDisconnectNodePinsCommand::UndoInternal(bool bFireEvents)
{
  nsDocument* pDocument = GetDocument();
  nsDocumentNodeManager* pManager = static_cast<nsDocumentNodeManager*>(pDocument->GetObjectManager());

  const nsPin* pOutput = pManager->GetOutputPinByName(m_pObjectSource, m_sSourcePin);
  if (pOutput == nullptr)
    return nsStatus("Connect Node: The given pin does not exist!");

  const nsPin* pInput = pManager->GetInputPinByName(m_pObjectTarget, m_sTargetPin);
  if (pInput == nullptr)
    return nsStatus("Connect Node: The given pin does not exist!");

  nsDocumentNodeManager::CanConnectResult res;
  NS_SUCCEED_OR_RETURN(pManager->CanConnect(m_pConnectionObject->GetType(), *pOutput, *pInput, res));

  pManager->Connect(m_pConnectionObject, *pOutput, *pInput);
  return nsStatus(NS_SUCCESS);
}


////////////////////////////////////////////////////////////////////////
// nsNodeCommands
////////////////////////////////////////////////////////////////////////

// static
nsStatus nsNodeCommands::AddAndConnectCommand(nsCommandHistory* pHistory, const nsRTTI* pConnectionType, const nsPin& sourcePin, const nsPin& targetPin)
{
  nsAddObjectCommand cmd;
  cmd.m_pType = pConnectionType;
  cmd.m_NewObjectGuid = nsUuid::MakeUuid();
  cmd.m_Index = -1;

  nsStatus res = pHistory->AddCommand(cmd);
  if (res.m_Result.Succeeded())
  {
    nsConnectNodePinsCommand connect;
    connect.m_ConnectionObject = cmd.m_NewObjectGuid;
    connect.m_ObjectSource = sourcePin.GetParent()->GetGuid();
    connect.m_ObjectTarget = targetPin.GetParent()->GetGuid();
    connect.m_sSourcePin = sourcePin.GetName();
    connect.m_sTargetPin = targetPin.GetName();

    res = pHistory->AddCommand(connect);
  }

  return res;
}

// static
nsStatus nsNodeCommands::DisconnectAndRemoveCommand(nsCommandHistory* pHistory, const nsUuid& connectionObject)
{
  nsDisconnectNodePinsCommand cmd;
  cmd.m_ConnectionObject = connectionObject;

  nsStatus res = pHistory->AddCommand(cmd);
  if (res.m_Result.Succeeded())
  {
    nsRemoveObjectCommand remove;
    remove.m_Object = cmd.m_ConnectionObject;

    res = pHistory->AddCommand(remove);
  }

  return res;
}
