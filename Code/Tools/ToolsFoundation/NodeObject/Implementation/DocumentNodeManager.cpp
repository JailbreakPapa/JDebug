#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <ToolsFoundation/Command/NodeCommands.h>
#include <ToolsFoundation/CommandHistory/CommandHistory.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/NodeObject/DocumentNodeManager.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdPin, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

////////////////////////////////////////////////////////////////////////
// wdDocumentNodeManager Internal
////////////////////////////////////////////////////////////////////////

struct DocumentNodeManager_NodeMetaData
{
  wdVec2 m_Pos = wdVec2::ZeroVector();
};
WD_DECLARE_REFLECTABLE_TYPE(WD_NO_LINKAGE, DocumentNodeManager_NodeMetaData);

// clang-format off
WD_BEGIN_STATIC_REFLECTED_TYPE(DocumentNodeManager_NodeMetaData, wdNoBase, 1, wdRTTIDefaultAllocator<DocumentNodeManager_NodeMetaData>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Node::Pos", m_Pos),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;
// clang-format on

struct DocumentNodeManager_ConnectionMetaData
{
  wdUuid m_Source;
  wdUuid m_Target;
  wdString m_SourcePin;
  wdString m_TargetPin;
};
WD_DECLARE_REFLECTABLE_TYPE(WD_NO_LINKAGE, DocumentNodeManager_ConnectionMetaData);

// clang-format off
WD_BEGIN_STATIC_REFLECTED_TYPE(DocumentNodeManager_ConnectionMetaData, wdNoBase, 1, wdRTTIDefaultAllocator<DocumentNodeManager_ConnectionMetaData>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Connection::Source", m_Source),
    WD_MEMBER_PROPERTY("Connection::Target", m_Target),
    WD_MEMBER_PROPERTY("Connection::SourcePin", m_SourcePin),    
    WD_MEMBER_PROPERTY("Connection::TargetPin", m_TargetPin),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;
// clang-format on

class DocumentNodeManager_DefaultConnection : public wdReflectedClass
{
  WD_ADD_DYNAMIC_REFLECTION(DocumentNodeManager_DefaultConnection, wdReflectedClass);
};

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(DocumentNodeManager_DefaultConnection, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

////////////////////////////////////////////////////////////////////////
// wdDocumentNodeManager
////////////////////////////////////////////////////////////////////////

wdDocumentNodeManager::wdDocumentNodeManager()
{
  m_ObjectEvents.AddEventHandler(wdMakeDelegate(&wdDocumentNodeManager::ObjectHandler, this));
  m_StructureEvents.AddEventHandler(wdMakeDelegate(&wdDocumentNodeManager::StructureEventHandler, this));
}

wdDocumentNodeManager::~wdDocumentNodeManager()
{
  m_ObjectEvents.RemoveEventHandler(wdMakeDelegate(&wdDocumentNodeManager::ObjectHandler, this));
  m_StructureEvents.RemoveEventHandler(wdMakeDelegate(&wdDocumentNodeManager::StructureEventHandler, this));
}

const wdRTTI* wdDocumentNodeManager::GetConnectionType() const
{
  return wdGetStaticRTTI<DocumentNodeManager_DefaultConnection>();
}

wdVec2 wdDocumentNodeManager::GetNodePos(const wdDocumentObject* pObject) const
{
  WD_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  auto it = m_ObjectToNode.Find(pObject->GetGuid());
  WD_ASSERT_DEV(it.IsValid(), "Can't get pos of objects that aren't nodes!");
  return it.Value().m_vPos;
}

const wdConnection& wdDocumentNodeManager::GetConnection(const wdDocumentObject* pObject) const
{
  WD_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  auto it = m_ObjectToConnection.Find(pObject->GetGuid());
  WD_ASSERT_DEV(it.IsValid(), "Can't get connection for objects that aren't connections!");
  return *it.Value();
}

const wdPin* wdDocumentNodeManager::GetInputPinByName(const wdDocumentObject* pObject, const char* szName) const
{
  WD_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  auto it = m_ObjectToNode.Find(pObject->GetGuid());
  WD_ASSERT_DEV(it.IsValid(), "Can't get input pins of objects that aren't nodes!");
  for (auto& pPin : it.Value().m_Inputs)
  {
    if (wdStringUtils::IsEqual(pPin->GetName(), szName))
      return pPin.Borrow();
  }
  return nullptr;
}

const wdPin* wdDocumentNodeManager::GetOutputPinByName(const wdDocumentObject* pObject, const char* szName) const
{
  WD_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  auto it = m_ObjectToNode.Find(pObject->GetGuid());
  WD_ASSERT_DEV(it.IsValid(), "Can't get input pins of objects that aren't nodes!");
  for (auto& pPin : it.Value().m_Outputs)
  {
    if (wdStringUtils::IsEqual(pPin->GetName(), szName))
      return pPin.Borrow();
  }
  return nullptr;
}

wdArrayPtr<const wdUniquePtr<const wdPin>> wdDocumentNodeManager::GetInputPins(const wdDocumentObject* pObject) const
{
  WD_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  auto it = m_ObjectToNode.Find(pObject->GetGuid());
  WD_ASSERT_DEV(it.IsValid(), "Can't get input pins of objects that aren't nodes!");
  return wdMakeArrayPtr((wdUniquePtr<const wdPin>*)it.Value().m_Inputs.GetData(), it.Value().m_Inputs.GetCount());
}

wdArrayPtr<const wdUniquePtr<const wdPin>> wdDocumentNodeManager::GetOutputPins(const wdDocumentObject* pObject) const
{
  WD_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  auto it = m_ObjectToNode.Find(pObject->GetGuid());
  WD_ASSERT_DEV(it.IsValid(), "Can't get input pins of objects that aren't nodes!");
  return wdMakeArrayPtr((wdUniquePtr<const wdPin>*)it.Value().m_Outputs.GetData(), it.Value().m_Outputs.GetCount());
}

bool wdDocumentNodeManager::IsNode(const wdDocumentObject* pObject) const
{
  WD_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  if (pObject == nullptr)
    return false;
  if (pObject == GetRootObject())
    return false;

  return InternalIsNode(pObject);
}

bool wdDocumentNodeManager::IsConnection(const wdDocumentObject* pObject) const
{
  WD_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  if (pObject == nullptr)
    return false;
  if (pObject == GetRootObject())
    return false;

  return InternalIsConnection(pObject);
}

wdArrayPtr<const wdConnection* const> wdDocumentNodeManager::GetConnections(const wdPin& pin) const
{
  auto it = m_Connections.Find(&pin);
  if (it.IsValid())
  {
    return it.Value();
  }

  return wdArrayPtr<const wdConnection* const>();
}

bool wdDocumentNodeManager::HasConnections(const wdPin& pin) const
{
  auto it = m_Connections.Find(&pin);
  return it.IsValid() && it.Value().IsEmpty() == false;
}

bool wdDocumentNodeManager::IsConnected(const wdPin& source, const wdPin& target) const
{
  auto it = m_Connections.Find(&source);
  if (it.IsValid())
  {
    for (auto pConnection : it.Value())
    {
      if (&pConnection->GetTargetPin() == &target)
        return true;
    }
  }

  return false;
}

wdStatus wdDocumentNodeManager::CanConnect(const wdRTTI* pObjectType, const wdPin& source, const wdPin& target, CanConnectResult& out_result) const
{
  out_result = CanConnectResult::ConnectNever;

  if (pObjectType == nullptr || pObjectType->IsDerivedFrom(GetConnectionType()) == false)
    return wdStatus("Invalid connection object type");

  if (source.m_Type != wdPin::Type::Output)
    return wdStatus("Source pin is not an output pin.");
  if (target.m_Type != wdPin::Type::Input)
    return wdStatus("Target pin is not an input pin.");

  if (source.m_pParent == target.m_pParent)
    return wdStatus("Nodes cannot be connect with themselves.");

  if (IsConnected(source, target))
    return wdStatus("Pins already connected.");

  return InternalCanConnect(source, target, out_result);
}

wdStatus wdDocumentNodeManager::CanDisconnect(const wdConnection* pConnection) const
{
  if (pConnection == nullptr)
    return wdStatus("Invalid connection");

  return InternalCanDisconnect(pConnection->GetSourcePin(), pConnection->GetTargetPin());
}

wdStatus wdDocumentNodeManager::CanDisconnect(const wdDocumentObject* pObject) const
{
  if (!IsConnection(pObject))
    return wdStatus("Invalid connection object");

  const wdConnection& connection = GetConnection(pObject);
  return InternalCanDisconnect(connection.GetSourcePin(), connection.GetTargetPin());
}

wdStatus wdDocumentNodeManager::CanMoveNode(const wdDocumentObject* pObject, const wdVec2& vPos) const
{
  WD_ASSERT_DEV(pObject != nullptr, "Invalid input!");
  if (!IsNode(pObject))
    return wdStatus("The given object is not a node!");

  return InternalCanMoveNode(pObject, vPos);
}

void wdDocumentNodeManager::Connect(const wdDocumentObject* pObject, const wdPin& source, const wdPin& target)
{
  wdDocumentNodeManager::CanConnectResult res = CanConnectResult::ConnectNever;
  WD_IGNORE_UNUSED(res);
  WD_ASSERT_DEBUG(CanConnect(pObject->GetType(), source, target, res).m_Result.Succeeded(), "Connect: Sanity check failed!");

  auto pConnection = WD_DEFAULT_NEW(wdConnection, source, target, pObject);
  m_ObjectToConnection.Insert(pObject->GetGuid(), pConnection);

  m_Connections[&source].PushBack(pConnection);
  m_Connections[&target].PushBack(pConnection);

  {
    wdDocumentNodeManagerEvent e(wdDocumentNodeManagerEvent::Type::AfterPinsConnected, pObject);
    m_NodeEvents.Broadcast(e);
  }
}

void wdDocumentNodeManager::Disconnect(const wdDocumentObject* pObject)
{
  auto it = m_ObjectToConnection.Find(pObject->GetGuid());
  WD_ASSERT_DEBUG(it.IsValid(), "Sanity check failed!");
  WD_ASSERT_DEBUG(CanDisconnect(pObject).m_Result.Succeeded(), "Disconnect: Sanity check failed!");

  {
    wdDocumentNodeManagerEvent e(wdDocumentNodeManagerEvent::Type::BeforePinsDisonnected, pObject);
    m_NodeEvents.Broadcast(e);
  }

  auto& pConnection = it.Value();
  const wdPin& source = pConnection->GetSourcePin();
  const wdPin& target = pConnection->GetTargetPin();
  m_Connections[&source].RemoveAndCopy(pConnection.Borrow());
  m_Connections[&target].RemoveAndCopy(pConnection.Borrow());

  m_ObjectToConnection.Remove(it);
}

void wdDocumentNodeManager::MoveNode(const wdDocumentObject* pObject, const wdVec2& vPos)
{
  WD_ASSERT_DEBUG(CanMoveNode(pObject, vPos).m_Result.Succeeded(), "MoveNode: Sanity check failed!");

  auto it = m_ObjectToNode.Find(pObject->GetGuid());
  WD_ASSERT_DEBUG(it.IsValid(), "Moveable node does not exist, CanMoveNode impl invalid!");
  it.Value().m_vPos = vPos;

  wdDocumentNodeManagerEvent e(wdDocumentNodeManagerEvent::Type::NodeMoved, pObject);
  m_NodeEvents.Broadcast(e);
}

void wdDocumentNodeManager::AttachMetaDataBeforeSaving(wdAbstractObjectGraph& ref_graph) const
{
  auto pNodeMetaDataType = wdGetStaticRTTI<DocumentNodeManager_NodeMetaData>();
  auto pConnectionMetaDataType = wdGetStaticRTTI<DocumentNodeManager_ConnectionMetaData>();

  wdRttiConverterContext context;
  wdRttiConverterWriter rttiConverter(&ref_graph, &context, true, true);

  for (auto it = ref_graph.GetAllNodes().GetIterator(); it.IsValid(); ++it)
  {
    auto* pAbstractObject = it.Value();
    const wdUuid& guid = pAbstractObject->GetGuid();

    {
      auto it2 = m_ObjectToNode.Find(guid);
      if (it2.IsValid())
      {
        const NodeInternal& node = it2.Value();

        DocumentNodeManager_NodeMetaData nodeMetaData;
        nodeMetaData.m_Pos = node.m_vPos;
        rttiConverter.AddProperties(pAbstractObject, pNodeMetaDataType, &nodeMetaData);
      }
    }

    {
      auto it2 = m_ObjectToConnection.Find(guid);
      if (it2.IsValid())
      {
        const wdConnection& connection = *it2.Value();
        const wdPin& sourcePin = connection.GetSourcePin();
        const wdPin& targetPin = connection.GetTargetPin();

        DocumentNodeManager_ConnectionMetaData connectionMetaData;
        connectionMetaData.m_Source = sourcePin.GetParent()->GetGuid();
        connectionMetaData.m_Target = targetPin.GetParent()->GetGuid();
        connectionMetaData.m_SourcePin = sourcePin.GetName();
        connectionMetaData.m_TargetPin = targetPin.GetName();
        rttiConverter.AddProperties(pAbstractObject, pConnectionMetaDataType, &connectionMetaData);
      }
    }
  }
}

void wdDocumentNodeManager::RestoreMetaDataAfterLoading(const wdAbstractObjectGraph& graph, bool bUndoable)
{
  wdCommandHistory* history = GetDocument()->GetCommandHistory();

  auto pNodeMetaDataType = wdGetStaticRTTI<DocumentNodeManager_NodeMetaData>();
  auto pConnectionMetaDataType = wdGetStaticRTTI<DocumentNodeManager_ConnectionMetaData>();

  wdRttiConverterContext context;
  wdRttiConverterReader rttiConverter(&graph, &context);

  for (auto it : graph.GetAllNodes())
  {
    auto pAbstractObject = it.Value();
    wdDocumentObject* pObject = GetObject(pAbstractObject->GetGuid());
    if (pObject == nullptr)
      continue;

    if (IsNode(pObject))
    {
      DocumentNodeManager_NodeMetaData nodeMetaData;
      rttiConverter.ApplyPropertiesToObject(pAbstractObject, pNodeMetaDataType, &nodeMetaData);

      if (CanMoveNode(pObject, nodeMetaData.m_Pos).m_Result.Succeeded())
      {
        if (bUndoable)
        {
          wdMoveNodeCommand move;
          move.m_Object = pObject->GetGuid();
          move.m_NewPos = nodeMetaData.m_Pos;
          history->AddCommand(move);
        }
        else
        {
          MoveNode(pObject, nodeMetaData.m_Pos);
        }
      }

      // Backwards compatibility to old file format
      if (auto pOldConnections = pAbstractObject->FindProperty("Node::Connections"))
      {
        WD_ASSERT_DEV(bUndoable == false, "Undo not supported for old file format");
        RestoreOldMetaDataAfterLoading(graph, *pOldConnections, pObject);
      }
    }
    else if (IsConnection(pObject))
    {
      DocumentNodeManager_ConnectionMetaData connectionMetaData;
      rttiConverter.ApplyPropertiesToObject(pAbstractObject, pConnectionMetaDataType, &connectionMetaData);

      wdDocumentObject* pSource = GetObject(connectionMetaData.m_Source);
      wdDocumentObject* pTarget = GetObject(connectionMetaData.m_Target);
      if (pSource == nullptr || pTarget == nullptr)
        continue;

      const wdPin* pSourcePin = GetOutputPinByName(pSource, connectionMetaData.m_SourcePin);
      const wdPin* pTargetPin = GetInputPinByName(pTarget, connectionMetaData.m_TargetPin);
      if (pSourcePin == nullptr || pTargetPin == nullptr)
        continue;

      wdDocumentNodeManager::CanConnectResult res;
      if (CanConnect(pObject->GetType(), *pSourcePin, *pTargetPin, res).m_Result.Succeeded())
      {
        if (bUndoable)
        {
          wdConnectNodePinsCommand cmd;
          cmd.m_ConnectionObject = pObject->GetGuid();
          cmd.m_ObjectSource = connectionMetaData.m_Source;
          cmd.m_ObjectTarget = connectionMetaData.m_Target;
          cmd.m_sSourcePin = connectionMetaData.m_SourcePin;
          cmd.m_sTargetPin = connectionMetaData.m_TargetPin;
          history->AddCommand(cmd);
        }
        else
        {
          Connect(pObject, *pSourcePin, *pTargetPin);
        }
      }
    }
  }
}

void wdDocumentNodeManager::GetMetaDataHash(const wdDocumentObject* pObject, wdUInt64& inout_uiHash) const
{
  if (IsNode(pObject))
  {
    // The node position is not hashed here since the hash is only used for asset transform
    // and for that the node position is irrelevant.
  }
  else if (IsConnection(pObject))
  {
    const wdConnection& connection = GetConnection(pObject);
    const wdPin& sourcePin = connection.GetSourcePin();
    const wdPin& targetPin = connection.GetTargetPin();

    inout_uiHash = wdHashingUtils::xxHash64(&sourcePin.GetParent()->GetGuid(), sizeof(wdUuid), inout_uiHash);
    inout_uiHash = wdHashingUtils::xxHash64(&targetPin.GetParent()->GetGuid(), sizeof(wdUuid), inout_uiHash);
    inout_uiHash = wdHashingUtils::xxHash64String(sourcePin.GetName(), inout_uiHash);
    inout_uiHash = wdHashingUtils::xxHash64String(targetPin.GetName(), inout_uiHash);
  }
}

bool wdDocumentNodeManager::CopySelectedObjects(wdAbstractObjectGraph& out_objectGraph) const
{
  const auto& selection = GetDocument()->GetSelectionManager()->GetSelection();

  if (selection.IsEmpty())
    return false;

  wdDocumentObjectConverterWriter writer(&out_objectGraph, this);

  wdHashSet<const wdDocumentObject*> copiedNodes;
  for (const wdDocumentObject* pObject : selection)
  {
    // Only add nodes here, connections are then collected below to ensure
    // that we always include only valid connections within the copied subgraph no matter if they are selected or not.
    if (IsNode(pObject))
    {
      // objects are required to be named root but this is not enforced or obvious by the interface.
      writer.AddObjectToGraph(pObject, "root");
      copiedNodes.Insert(pObject);
    }
  }

  wdHashSet<const wdDocumentObject*> copiedConnections;
  for (const wdDocumentObject* pNodeObject : selection)
  {
    if (IsNode(pNodeObject) == false)
      continue;

    auto outputs = GetOutputPins(pNodeObject);
    for (auto& pSourcePin : outputs)
    {
      auto connections = GetConnections(*pSourcePin);
      for (const wdConnection* pConnection : connections)
      {
        const wdDocumentObject* pConnectionObject = pConnection->GetParent();

        WD_ASSERT_DEV(pSourcePin == &pConnection->GetSourcePin(), "");
        if (copiedConnections.Contains(pConnectionObject) == false && copiedNodes.Contains(pConnection->GetTargetPin().GetParent()))
        {
          writer.AddObjectToGraph(pConnectionObject, "root");
          copiedConnections.Insert(pConnectionObject);
        }
      }
    }
  }

  AttachMetaDataBeforeSaving(out_objectGraph);

  return true;
}

bool wdDocumentNodeManager::PasteObjects(const wdArrayPtr<wdDocument::PasteInfo>& info, const wdAbstractObjectGraph& objectGraph, const wdVec2& vPickedPosition, bool bAllowPickedPosition)
{
  bool bAddedAll = true;
  wdDeque<const wdDocumentObject*> AddedObjects;

  for (const auto& pi : info)
  {
    // only add nodes that are allowed to be added
    if (CanAdd(pi.m_pObject->GetTypeAccessor().GetType(), nullptr, "Children", pi.m_Index).m_Result.Succeeded())
    {
      AddedObjects.PushBack(pi.m_pObject);
      AddObject(pi.m_pObject, nullptr, "Children", pi.m_Index);
    }
    else
    {
      bAddedAll = false;
    }
  }

  RestoreMetaDataAfterLoading(objectGraph, true);

  if (!AddedObjects.IsEmpty() && bAllowPickedPosition)
  {
    wdCommandHistory* history = GetDocument()->GetCommandHistory();

    wdVec2 vAvgPos(0);
    wdUInt32 nodeCount = 0;
    for (const wdDocumentObject* pObject : AddedObjects)
    {
      if (IsNode(pObject))
      {
        vAvgPos += GetNodePos(pObject);
        ++nodeCount;
      }
    }

    vAvgPos /= (float)nodeCount;
    const wdVec2 vMoveNode = -vAvgPos + vPickedPosition;

    for (const wdDocumentObject* pObject : AddedObjects)
    {
      if (IsNode(pObject))
      {
        wdMoveNodeCommand move;
        move.m_Object = pObject->GetGuid();
        move.m_NewPos = GetNodePos(pObject) + vMoveNode;
        history->AddCommand(move);
      }
    }

    if (!bAddedAll)
    {
      wdLog::Info("[EditorStatus]Not all nodes were allowed to be added to the document");
    }
  }

  GetDocument()->GetSelectionManager()->SetSelection(AddedObjects);
  return true;
}

bool wdDocumentNodeManager::CanReachNode(const wdDocumentObject* pSource, const wdDocumentObject* pTarget, wdSet<const wdDocumentObject*>& Visited) const
{
  if (pSource == pTarget)
    return true;

  if (Visited.Contains(pSource))
    return false;

  Visited.Insert(pSource);

  auto outputs = GetOutputPins(pSource);
  for (auto& pSourcePin : outputs)
  {
    auto connections = GetConnections(*pSourcePin);
    for (const wdConnection* pConnection : connections)
    {
      if (CanReachNode(pConnection->GetTargetPin().GetParent(), pTarget, Visited))
        return true;
    }
  }

  return false;
}


bool wdDocumentNodeManager::WouldConnectionCreateCircle(const wdPin& source, const wdPin& target) const
{
  const wdDocumentObject* pSourceNode = source.GetParent();
  const wdDocumentObject* pTargetNode = target.GetParent();
  wdSet<const wdDocumentObject*> Visited;

  return CanReachNode(pTargetNode, pSourceNode, Visited);
}

bool wdDocumentNodeManager::InternalIsNode(const wdDocumentObject* pObject) const
{
  return true;
}

bool wdDocumentNodeManager::InternalIsConnection(const wdDocumentObject* pObject) const
{
  auto pType = pObject->GetTypeAccessor().GetType();
  return pType->IsDerivedFrom(GetConnectionType());
}

wdStatus wdDocumentNodeManager::InternalCanConnect(const wdPin& source, const wdPin& target, CanConnectResult& out_Result) const
{
  out_Result = CanConnectResult::ConnectNtoN;
  return wdStatus(WD_SUCCESS);
}

void wdDocumentNodeManager::ObjectHandler(const wdDocumentObjectEvent& e)
{
  switch (e.m_EventType)
  {
    case wdDocumentObjectEvent::Type::AfterObjectCreated:
    {
      if (IsNode(e.m_pObject))
      {
        WD_ASSERT_DEBUG(!m_ObjectToNode.Contains(e.m_pObject->GetGuid()), "Sanity check failed!");
        m_ObjectToNode[e.m_pObject->GetGuid()] = NodeInternal();
        InternalCreatePins(e.m_pObject, m_ObjectToNode[e.m_pObject->GetGuid()]);
        // TODO: Sanity check pins (duplicate names etc).
      }
      else if (IsConnection(e.m_pObject))
      {
        // Nothing to do here: Map entries are created in Connect method.
      }
    }
    break;
    case wdDocumentObjectEvent::Type::BeforeObjectDestroyed:
    {
      if (IsNode(e.m_pObject))
      {
        auto it = m_ObjectToNode.Find(e.m_pObject->GetGuid());
        WD_ASSERT_DEBUG(it.IsValid(), "Sanity check failed!");

        m_ObjectToNode.Remove(it);
      }
      else if (IsConnection(e.m_pObject))
      {
        // Nothing to do here: Map entries are removed in Disconnect method.
      }
    }
    break;
  }
}

void wdDocumentNodeManager::StructureEventHandler(const wdDocumentObjectStructureEvent& e)
{
  switch (e.m_EventType)
  {
    case wdDocumentObjectStructureEvent::Type::BeforeObjectAdded:
    {
      if (IsNode(e.m_pObject))
      {
        wdDocumentNodeManagerEvent e2(wdDocumentNodeManagerEvent::Type::BeforeNodeAdded, e.m_pObject);
        m_NodeEvents.Broadcast(e2);
      }
    }
    break;
    case wdDocumentObjectStructureEvent::Type::AfterObjectAdded:
    {
      if (IsNode(e.m_pObject))
      {
        wdDocumentNodeManagerEvent e2(wdDocumentNodeManagerEvent::Type::AfterNodeAdded, e.m_pObject);
        m_NodeEvents.Broadcast(e2);
      }
    }
    break;
    case wdDocumentObjectStructureEvent::Type::BeforeObjectRemoved:
    {
      if (IsNode(e.m_pObject))
      {
        wdDocumentNodeManagerEvent e2(wdDocumentNodeManagerEvent::Type::BeforeNodeRemoved, e.m_pObject);
        m_NodeEvents.Broadcast(e2);
      }
    }
    break;
    case wdDocumentObjectStructureEvent::Type::AfterObjectRemoved:
    {
      if (IsNode(e.m_pObject))
      {
        wdDocumentNodeManagerEvent e2(wdDocumentNodeManagerEvent::Type::AfterNodeRemoved, e.m_pObject);
        m_NodeEvents.Broadcast(e2);
      }
    }
    break;

    default:
      break;
  }
}

void wdDocumentNodeManager::RestoreOldMetaDataAfterLoading(const wdAbstractObjectGraph& graph, const wdAbstractObjectNode::Property& connectionsProperty, const wdDocumentObject* pSourceObject)
{
  if (connectionsProperty.m_Value.IsA<wdVariantArray>() == false)
    return;

  const wdVariantArray& array = connectionsProperty.m_Value.Get<wdVariantArray>();
  for (const wdVariant& var : array)
  {
    if (var.IsA<wdUuid>() == false)
      continue;

    auto pOldConnectionAbstractObject = graph.GetNode(var.Get<wdUuid>());
    auto pTargetProperty = pOldConnectionAbstractObject->FindProperty("Target");
    if (pTargetProperty == nullptr || pTargetProperty->m_Value.IsA<wdUuid>() == false)
      continue;

    wdDocumentObject* pTargetObject = GetObject(pTargetProperty->m_Value.Get<wdUuid>());
    if (pTargetObject == nullptr)
      continue;

    auto pSourcePinProperty = pOldConnectionAbstractObject->FindProperty("SourcePin");
    if (pSourcePinProperty == nullptr || pSourcePinProperty->m_Value.IsA<wdString>() == false)
      continue;

    auto pTargetPinProperty = pOldConnectionAbstractObject->FindProperty("TargetPin");
    if (pTargetPinProperty == nullptr || pTargetPinProperty->m_Value.IsA<wdString>() == false)
      continue;

    const wdPin* pSourcePin = GetOutputPinByName(pSourceObject, pSourcePinProperty->m_Value.Get<wdString>());
    const wdPin* pTargetPin = GetInputPinByName(pTargetObject, pTargetPinProperty->m_Value.Get<wdString>());
    if (pSourcePin == nullptr || pTargetPin == nullptr)
      continue;

    const wdRTTI* pConnectionType = GetConnectionType();
    wdDocumentNodeManager::CanConnectResult res;
    if (CanConnect(pConnectionType, *pSourcePin, *pTargetPin, res).m_Result.Succeeded())
    {
      wdUuid ObjectGuid;
      ObjectGuid.CreateNewUuid();
      wdDocumentObject* pConnectionObject = CreateObject(pConnectionType, ObjectGuid);

      AddObject(pConnectionObject, nullptr, "", -1);

      Connect(pConnectionObject, *pSourcePin, *pTargetPin);
    }
  }
}
