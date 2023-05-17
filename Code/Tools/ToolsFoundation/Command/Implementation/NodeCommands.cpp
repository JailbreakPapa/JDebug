#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Command/NodeCommands.h>
#include <ToolsFoundation/Command/TreeCommands.h>
#include <ToolsFoundation/NodeObject/DocumentNodeManager.h>

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdRemoveNodeCommand, 1, wdRTTIDefaultAllocator<wdRemoveNodeCommand>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("ObjectGuid", m_Object),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdMoveNodeCommand, 1, wdRTTIDefaultAllocator<wdMoveNodeCommand>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("ObjectGuid", m_Object),
    WD_MEMBER_PROPERTY("NewPos", m_NewPos),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdConnectNodePinsCommand, 1, wdRTTIDefaultAllocator<wdConnectNodePinsCommand>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("ConnectionGuid", m_ConnectionObject),
    WD_MEMBER_PROPERTY("SourceGuid", m_ObjectSource),
    WD_MEMBER_PROPERTY("TargetGuid", m_ObjectTarget),
    WD_MEMBER_PROPERTY("SourcePin", m_sSourcePin),
    WD_MEMBER_PROPERTY("TargetPin", m_sTargetPin),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdDisconnectNodePinsCommand, 1, wdRTTIDefaultAllocator<wdDisconnectNodePinsCommand>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("ConnectionGuid", m_ConnectionObject),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

////////////////////////////////////////////////////////////////////////
// wdRemoveNodeCommand
////////////////////////////////////////////////////////////////////////

wdRemoveNodeCommand::wdRemoveNodeCommand() = default;

wdStatus wdRemoveNodeCommand::DoInternal(bool bRedo)
{
  wdDocument* pDocument = GetDocument();
  wdDocumentNodeManager* pManager = static_cast<wdDocumentNodeManager*>(pDocument->GetObjectManager());

  auto RemoveConnections = [&](const wdPin& pin) {
    while (true)
    {
      auto connections = pManager->GetConnections(pin);

      if (connections.IsEmpty())
        break;

      wdDisconnectNodePinsCommand cmd;
      cmd.m_ConnectionObject = connections[0]->GetParent()->GetGuid();
      wdStatus res = AddSubCommand(cmd);
      if (res.m_Result.Succeeded())
      {
        wdRemoveObjectCommand remove;
        remove.m_Object = cmd.m_ConnectionObject;
        res = AddSubCommand(remove);
      }

      WD_SUCCEED_OR_RETURN(res);
    }
    return wdStatus(WD_SUCCESS);
  };

  if (!bRedo)
  {
    m_pObject = pManager->GetObject(m_Object);
    if (m_pObject == nullptr)
      return wdStatus("Remove Node: The given object does not exist!");

    auto inputs = pManager->GetInputPins(m_pObject);
    for (auto& pPinTarget : inputs)
    {
      WD_SUCCEED_OR_RETURN(RemoveConnections(*pPinTarget));
    }

    auto outputs = pManager->GetOutputPins(m_pObject);
    for (auto& pPinSource : outputs)
    {
      WD_SUCCEED_OR_RETURN(RemoveConnections(*pPinSource));
    }

    wdRemoveObjectCommand cmd;
    cmd.m_Object = m_Object;
    auto res = AddSubCommand(cmd);
    if (res.m_Result.Failed())
    {
      return res;
    }
  }
  return wdStatus(WD_SUCCESS);
}

wdStatus wdRemoveNodeCommand::UndoInternal(bool bFireEvents)
{
  WD_ASSERT_DEV(bFireEvents, "This command does not support temporary commands");
  return wdStatus(WD_SUCCESS);
}

void wdRemoveNodeCommand::CleanupInternal(CommandState state) {}


////////////////////////////////////////////////////////////////////////
// wdMoveObjectCommand
////////////////////////////////////////////////////////////////////////

wdMoveNodeCommand::wdMoveNodeCommand() = default;

wdStatus wdMoveNodeCommand::DoInternal(bool bRedo)
{
  wdDocument* pDocument = GetDocument();
  wdDocumentNodeManager* pManager = static_cast<wdDocumentNodeManager*>(pDocument->GetObjectManager());

  if (!bRedo)
  {
    m_pObject = pDocument->GetObjectManager()->GetObject(m_Object);
    if (m_pObject == nullptr)
      return wdStatus("Move Node: The given object does not exist!");

    m_vOldPos = pManager->GetNodePos(m_pObject);
    WD_SUCCEED_OR_RETURN(pManager->CanMoveNode(m_pObject, m_NewPos));
  }

  pManager->MoveNode(m_pObject, m_NewPos);
  return wdStatus(WD_SUCCESS);
}

wdStatus wdMoveNodeCommand::UndoInternal(bool bFireEvents)
{
  wdDocument* pDocument = GetDocument();
  wdDocumentNodeManager* pManager = static_cast<wdDocumentNodeManager*>(pDocument->GetObjectManager());
  WD_ASSERT_DEV(bFireEvents, "This command does not support temporary commands");

  WD_SUCCEED_OR_RETURN(pManager->CanMoveNode(m_pObject, m_vOldPos));

  pManager->MoveNode(m_pObject, m_vOldPos);

  return wdStatus(WD_SUCCESS);
}


////////////////////////////////////////////////////////////////////////
// wdConnectNodePinsCommand
////////////////////////////////////////////////////////////////////////

wdConnectNodePinsCommand::wdConnectNodePinsCommand() = default;

wdStatus wdConnectNodePinsCommand::DoInternal(bool bRedo)
{
  wdDocument* pDocument = GetDocument();
  wdDocumentNodeManager* pManager = static_cast<wdDocumentNodeManager*>(pDocument->GetObjectManager());

  if (!bRedo)
  {
    m_pConnectionObject = pManager->GetObject(m_ConnectionObject);
    if (!pManager->IsConnection(m_pConnectionObject))
      return wdStatus("Connect Node Pins: The given connection object is not valid connection!");

    m_pObjectSource = pManager->GetObject(m_ObjectSource);
    if (m_pObjectSource == nullptr)
      return wdStatus("Connect Node Pins: The given node does not exist!");
    m_pObjectTarget = pManager->GetObject(m_ObjectTarget);
    if (m_pObjectTarget == nullptr)
      return wdStatus("Connect Node Pins: The given node does not exist!");
  }

  const wdPin* pOutput = pManager->GetOutputPinByName(m_pObjectSource, m_sSourcePin);
  if (pOutput == nullptr)
    return wdStatus("Connect Node Pins: The given pin does not exist!");

  const wdPin* pInput = pManager->GetInputPinByName(m_pObjectTarget, m_sTargetPin);
  if (pInput == nullptr)
    return wdStatus("Connect Node Pins: The given pin does not exist!");

  wdDocumentNodeManager::CanConnectResult res;
  WD_SUCCEED_OR_RETURN(pManager->CanConnect(m_pConnectionObject->GetType(), *pOutput, *pInput, res));

  pManager->Connect(m_pConnectionObject, *pOutput, *pInput);
  return wdStatus(WD_SUCCESS);
}

wdStatus wdConnectNodePinsCommand::UndoInternal(bool bFireEvents)
{
  wdDocument* pDocument = GetDocument();
  wdDocumentNodeManager* pManager = static_cast<wdDocumentNodeManager*>(pDocument->GetObjectManager());

  WD_SUCCEED_OR_RETURN(pManager->CanDisconnect(m_pConnectionObject));

  pManager->Disconnect(m_pConnectionObject);
  return wdStatus(WD_SUCCESS);
}


////////////////////////////////////////////////////////////////////////
// wdDisconnectNodePinsCommand
////////////////////////////////////////////////////////////////////////

wdDisconnectNodePinsCommand::wdDisconnectNodePinsCommand() = default;

wdStatus wdDisconnectNodePinsCommand::DoInternal(bool bRedo)
{
  wdDocument* pDocument = GetDocument();
  wdDocumentNodeManager* pManager = static_cast<wdDocumentNodeManager*>(pDocument->GetObjectManager());

  if (!bRedo)
  {
    m_pConnectionObject = pManager->GetObject(m_ConnectionObject);
    if (!pManager->IsConnection(m_pConnectionObject))
      return wdStatus("Disconnect Node Pins: The given connection object is not valid connection!");

    WD_SUCCEED_OR_RETURN(pManager->CanRemove(m_pConnectionObject));

    const wdConnection& connection = pManager->GetConnection(m_pConnectionObject);
    const wdPin& pinSource = connection.GetSourcePin();
    const wdPin& pinTarget = connection.GetTargetPin();

    m_pObjectSource = pinSource.GetParent();
    m_pObjectTarget = pinTarget.GetParent();
    m_sSourcePin = pinSource.GetName();
    m_sTargetPin = pinTarget.GetName();
  }

  WD_SUCCEED_OR_RETURN(pManager->CanDisconnect(m_pConnectionObject));

  pManager->Disconnect(m_pConnectionObject);

  return wdStatus(WD_SUCCESS);
}

wdStatus wdDisconnectNodePinsCommand::UndoInternal(bool bFireEvents)
{
  wdDocument* pDocument = GetDocument();
  wdDocumentNodeManager* pManager = static_cast<wdDocumentNodeManager*>(pDocument->GetObjectManager());

  const wdPin* pOutput = pManager->GetOutputPinByName(m_pObjectSource, m_sSourcePin);
  if (pOutput == nullptr)
    return wdStatus("Connect Node: The given pin does not exist!");

  const wdPin* pInput = pManager->GetInputPinByName(m_pObjectTarget, m_sTargetPin);
  if (pInput == nullptr)
    return wdStatus("Connect Node: The given pin does not exist!");

  wdDocumentNodeManager::CanConnectResult res;
  WD_SUCCEED_OR_RETURN(pManager->CanConnect(m_pConnectionObject->GetType(), *pOutput, *pInput, res));

  pManager->Connect(m_pConnectionObject, *pOutput, *pInput);
  return wdStatus(WD_SUCCESS);
}


////////////////////////////////////////////////////////////////////////
// wdNodeCommands
////////////////////////////////////////////////////////////////////////

// static
wdStatus wdNodeCommands::AddAndConnectCommand(wdCommandHistory* pHistory, const wdRTTI* pConnectionType, const wdPin& sourcePin, const wdPin& targetPin)
{
  wdAddObjectCommand cmd;
  cmd.m_pType = pConnectionType;
  cmd.m_NewObjectGuid.CreateNewUuid();
  cmd.m_Index = -1;

  wdStatus res = pHistory->AddCommand(cmd);
  if (res.m_Result.Succeeded())
  {
    wdConnectNodePinsCommand connect;
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
wdStatus wdNodeCommands::DisconnectAndRemoveCommand(wdCommandHistory* pHistory, const wdUuid& connectionObject)
{
  wdDisconnectNodePinsCommand cmd;
  cmd.m_ConnectionObject = connectionObject;

  wdStatus res = pHistory->AddCommand(cmd);
  if (res.m_Result.Succeeded())
  {
    wdRemoveObjectCommand remove;
    remove.m_Object = cmd.m_ConnectionObject;

    res = pHistory->AddCommand(remove);
  }

  return res;
}
