#pragma once

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Types/Status.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

class wdPin;
class wdConnection;

struct WD_TOOLSFOUNDATION_DLL wdDocumentNodeManagerEvent
{
  enum class Type
  {
    NodeMoved,
    AfterPinsConnected,
    BeforePinsDisonnected,
    BeforePinsChanged, // todo
    AfterPinsChanged,  // todo
    BeforeNodeAdded,
    AfterNodeAdded,
    BeforeNodeRemoved,
    AfterNodeRemoved,
  };

  wdDocumentNodeManagerEvent(Type eventType, const wdDocumentObject* pObject = nullptr)
    : m_EventType(eventType)
    , m_pObject(pObject)
  {
  }

  Type m_EventType;
  const wdDocumentObject* m_pObject;
};

class wdConnection
{
public:
  const wdPin& GetSourcePin() const { return m_SourcePin; }
  const wdPin& GetTargetPin() const { return m_TargetPin; }
  const wdDocumentObject* GetParent() const { return m_pParent; }

private:
  friend class wdDocumentNodeManager;

  wdConnection(const wdPin& sourcePin, const wdPin& targetPin, const wdDocumentObject* pParent)
    : m_SourcePin(sourcePin)
    , m_TargetPin(targetPin)
    , m_pParent(pParent)
  {
  }

  const wdPin& m_SourcePin;
  const wdPin& m_TargetPin;
  const wdDocumentObject* m_pParent = nullptr;
};

class WD_TOOLSFOUNDATION_DLL wdPin : public wdReflectedClass
{
  WD_ADD_DYNAMIC_REFLECTION(wdPin, wdReflectedClass);

public:
  enum class Type
  {
    Input,
    Output
  };

  enum class Shape
  {
    Circle,
    Rect,
    RoundRect,
    Default = Circle
  };

  wdPin(Type type, const char* szName, const wdColorGammaUB& color, const wdDocumentObject* pObject)
    : m_Type(type)
    , m_Color(color)
    , m_sName(szName)
    , m_pParent(pObject)
  {
  }

  Shape m_Shape = Shape::Default;

  Type GetType() const { return m_Type; }
  const char* GetName() const { return m_sName; }
  const wdColorGammaUB& GetColor() const { return m_Color; }
  const wdDocumentObject* GetParent() const { return m_pParent; }

private:
  friend class wdDocumentNodeManager;

  Type m_Type;
  wdColorGammaUB m_Color;
  wdString m_sName;
  const wdDocumentObject* m_pParent = nullptr;
};

class WD_TOOLSFOUNDATION_DLL wdDocumentNodeManager : public wdDocumentObjectManager
{
public:
  wdEvent<const wdDocumentNodeManagerEvent&> m_NodeEvents;

  wdDocumentNodeManager();
  virtual ~wdDocumentNodeManager();

  virtual const wdRTTI* GetConnectionType() const;

  wdVec2 GetNodePos(const wdDocumentObject* pObject) const;
  const wdConnection& GetConnection(const wdDocumentObject* pObject) const;

  const wdPin* GetInputPinByName(const wdDocumentObject* pObject, const char* szName) const;
  const wdPin* GetOutputPinByName(const wdDocumentObject* pObject, const char* szName) const;
  wdArrayPtr<const wdUniquePtr<const wdPin>> GetInputPins(const wdDocumentObject* pObject) const;
  wdArrayPtr<const wdUniquePtr<const wdPin>> GetOutputPins(const wdDocumentObject* pObject) const;

  enum class CanConnectResult
  {
    ConnectNever, ///< Pins can't be connected
    Connect1to1,  ///< Output pin can have 1 outgoing connection, Input pin can have 1 incoming connection
    Connect1toN,  ///< Output pin can have 1 outgoing connection, Input pin can have N incoming connections
    ConnectNto1,  ///< Output pin can have N outgoing connections, Input pin can have 1 incoming connection
    ConnectNtoN,  ///< Output pin can have N outgoing connections, Input pin can have N incoming connections
  };

  bool IsNode(const wdDocumentObject* pObject) const;
  bool IsConnection(const wdDocumentObject* pObject) const;

  wdArrayPtr<const wdConnection* const> GetConnections(const wdPin& pin) const;
  bool HasConnections(const wdPin& pin) const;
  bool IsConnected(const wdPin& source, const wdPin& target) const;
  wdStatus CanConnect(const wdRTTI* pObjectType, const wdPin& source, const wdPin& target, CanConnectResult& ref_result) const;
  wdStatus CanDisconnect(const wdConnection* pConnection) const;
  wdStatus CanDisconnect(const wdDocumentObject* pObject) const;
  wdStatus CanMoveNode(const wdDocumentObject* pObject, const wdVec2& vPos) const;

  void Connect(const wdDocumentObject* pObject, const wdPin& source, const wdPin& target);
  void Disconnect(const wdDocumentObject* pObject);
  void MoveNode(const wdDocumentObject* pObject, const wdVec2& vPos);

  void AttachMetaDataBeforeSaving(wdAbstractObjectGraph& ref_graph) const;
  void RestoreMetaDataAfterLoading(const wdAbstractObjectGraph& graph, bool bUndoable);

  void GetMetaDataHash(const wdDocumentObject* pObject, wdUInt64& inout_uiHash) const;
  bool CopySelectedObjects(wdAbstractObjectGraph& out_objectGraph) const;
  bool PasteObjects(const wdArrayPtr<wdDocument::PasteInfo>& info, const wdAbstractObjectGraph& objectGraph, const wdVec2& vPickedPosition, bool bAllowPickedPosition);

protected:
  /// \brief Tests whether pTarget can be reached from pSource by following the pin connections
  bool CanReachNode(const wdDocumentObject* pSource, const wdDocumentObject* pTarget, wdSet<const wdDocumentObject*>& Visited) const;

  /// \brief Returns true if adding a connection between the two pins would create a circular graph
  bool WouldConnectionCreateCircle(const wdPin& source, const wdPin& target) const;

  struct NodeInternal
  {
    wdVec2 m_vPos = wdVec2::ZeroVector();
    wdHybridArray<wdUniquePtr<wdPin>, 6> m_Inputs;
    wdHybridArray<wdUniquePtr<wdPin>, 6> m_Outputs;
  };

private:
  virtual bool InternalIsNode(const wdDocumentObject* pObject) const;
  virtual bool InternalIsConnection(const wdDocumentObject* pObject) const;
  virtual wdStatus InternalCanConnect(const wdPin& source, const wdPin& target, CanConnectResult& out_Result) const;
  virtual wdStatus InternalCanDisconnect(const wdPin& source, const wdPin& target) const { return wdStatus(WD_SUCCESS); }
  virtual wdStatus InternalCanMoveNode(const wdDocumentObject* pObject, const wdVec2& vPos) const { return wdStatus(WD_SUCCESS); }
  virtual void InternalCreatePins(const wdDocumentObject* pObject, NodeInternal& node) = 0;

  void ObjectHandler(const wdDocumentObjectEvent& e);
  void StructureEventHandler(const wdDocumentObjectStructureEvent& e);

  void RestoreOldMetaDataAfterLoading(const wdAbstractObjectGraph& graph, const wdAbstractObjectNode::Property& connectionsProperty, const wdDocumentObject* pSourceObject);

private:
  wdHashTable<wdUuid, NodeInternal> m_ObjectToNode;
  wdHashTable<wdUuid, wdUniquePtr<wdConnection>> m_ObjectToConnection;
  wdMap<const wdPin*, wdHybridArray<const wdConnection*, 6>> m_Connections;
};
