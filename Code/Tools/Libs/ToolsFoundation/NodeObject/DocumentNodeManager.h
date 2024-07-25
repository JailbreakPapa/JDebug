#pragma once

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Types/Status.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

class nsPin;
class nsConnection;

struct NS_TOOLSFOUNDATION_DLL nsDocumentNodeManagerEvent
{
  enum class Type
  {
    NodeMoved,
    AfterPinsConnected,
    BeforePinsDisonnected,
    BeforePinsChanged,
    AfterPinsChanged,
    BeforeNodeAdded,
    AfterNodeAdded,
    BeforeNodeRemoved,
    AfterNodeRemoved,
  };

  nsDocumentNodeManagerEvent(Type eventType, const nsDocumentObject* pObject = nullptr)
    : m_EventType(eventType)
    , m_pObject(pObject)
  {
  }

  Type m_EventType;
  const nsDocumentObject* m_pObject;
};

class nsConnection
{
public:
  const nsPin& GetSourcePin() const { return m_SourcePin; }
  const nsPin& GetTargetPin() const { return m_TargetPin; }
  const nsDocumentObject* GetParent() const { return m_pParent; }

private:
  friend class nsDocumentNodeManager;

  nsConnection(const nsPin& sourcePin, const nsPin& targetPin, const nsDocumentObject* pParent)
    : m_SourcePin(sourcePin)
    , m_TargetPin(targetPin)
    , m_pParent(pParent)
  {
  }

  const nsPin& m_SourcePin;
  const nsPin& m_TargetPin;
  const nsDocumentObject* m_pParent = nullptr;
};

class NS_TOOLSFOUNDATION_DLL nsPin : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsPin, nsReflectedClass);

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
    Arrow,
    Default = Circle
  };

  nsPin(Type type, nsStringView sName, const nsColorGammaUB& color, const nsDocumentObject* pObject)
    : m_Type(type)
    , m_Color(color)
    , m_sName(sName)
    , m_pParent(pObject)
  {
  }

  Shape m_Shape = Shape::Default;

  Type GetType() const { return m_Type; }
  const char* GetName() const { return m_sName; }
  const nsColorGammaUB& GetColor() const { return m_Color; }
  const nsDocumentObject* GetParent() const { return m_pParent; }

private:
  friend class nsDocumentNodeManager;

  Type m_Type;
  nsColorGammaUB m_Color;
  nsString m_sName;
  const nsDocumentObject* m_pParent = nullptr;
};

//////////////////////////////////////////////////////////////////////////

struct nsNodePropertyValue
{
  nsHashedString m_sPropertyName;
  nsVariant m_Value;
};

/// \brief Describes a template that will be used to create new nodes. In most cases this only contains the type
/// but it can also contain properties that are pre-filled when the node is created.
///
/// For example in visual script this allows us to have one generic node type for setting reflected properties
/// but we can expose all relevant reflected properties in the node creation menu so the user does not need to fill out the property name manually.
struct nsNodeCreationTemplate
{
  const nsRTTI* m_pType = nullptr;
  nsStringView m_sTypeName;
  nsHashedString m_sCategory;
  nsArrayPtr<const nsNodePropertyValue> m_PropertyValues;
};

//////////////////////////////////////////////////////////////////////////

/// \brief Base class for all node connections. Derive from this class and overwrite nsDocumentNodeManager::GetConnectionType
/// if you need custom properties for connections.
class NS_TOOLSFOUNDATION_DLL nsDocumentObject_ConnectionBase : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsDocumentObject_ConnectionBase, nsReflectedClass);

public:
  nsUuid m_Source;
  nsUuid m_Target;
  nsString m_SourcePin;
  nsString m_TargetPin;
};

//////////////////////////////////////////////////////////////////////////

class NS_TOOLSFOUNDATION_DLL nsDocumentNodeManager : public nsDocumentObjectManager
{
public:
  nsEvent<const nsDocumentNodeManagerEvent&> m_NodeEvents;

  nsDocumentNodeManager();
  virtual ~nsDocumentNodeManager();

  /// \brief For node documents this function is called instead of GetCreateableTypes to get a list for the node creation menu.
  ///
  /// \see nsNodeCreationTemplate
  virtual void GetNodeCreationTemplates(nsDynamicArray<nsNodeCreationTemplate>& out_templates) const;

  virtual const nsRTTI* GetConnectionType() const;

  nsVec2 GetNodePos(const nsDocumentObject* pObject) const;
  const nsConnection& GetConnection(const nsDocumentObject* pObject) const;
  const nsConnection* GetConnectionIfExists(const nsDocumentObject* pObject) const;

  const nsPin* GetInputPinByName(const nsDocumentObject* pObject, nsStringView sName) const;
  const nsPin* GetOutputPinByName(const nsDocumentObject* pObject, nsStringView sName) const;
  nsArrayPtr<const nsUniquePtr<const nsPin>> GetInputPins(const nsDocumentObject* pObject) const;
  nsArrayPtr<const nsUniquePtr<const nsPin>> GetOutputPins(const nsDocumentObject* pObject) const;

  enum class CanConnectResult
  {
    ConnectNever, ///< Pins can't be connected
    Connect1to1,  ///< Output pin can have 1 outgoing connection, Input pin can have 1 incoming connection
    Connect1toN,  ///< Output pin can have 1 outgoing connection, Input pin can have N incoming connections
    ConnectNto1,  ///< Output pin can have N outgoing connections, Input pin can have 1 incoming connection
    ConnectNtoN,  ///< Output pin can have N outgoing connections, Input pin can have N incoming connections
  };

  bool IsNode(const nsDocumentObject* pObject) const;
  bool IsConnection(const nsDocumentObject* pObject) const;
  bool IsDynamicPinProperty(const nsDocumentObject* pObject, const nsAbstractProperty* pProp) const;

  nsArrayPtr<const nsConnection* const> GetConnections(const nsPin& pin) const;
  bool HasConnections(const nsPin& pin) const;
  bool IsConnected(const nsPin& source, const nsPin& target) const;

  nsStatus CanConnect(const nsRTTI* pObjectType, const nsPin& source, const nsPin& target, CanConnectResult& ref_result) const;
  nsStatus CanDisconnect(const nsConnection* pConnection) const;
  nsStatus CanDisconnect(const nsDocumentObject* pObject) const;
  nsStatus CanMoveNode(const nsDocumentObject* pObject, const nsVec2& vPos) const;

  void Connect(const nsDocumentObject* pObject, const nsPin& source, const nsPin& target);
  void Disconnect(const nsDocumentObject* pObject);
  void MoveNode(const nsDocumentObject* pObject, const nsVec2& vPos);

  void AttachMetaDataBeforeSaving(nsAbstractObjectGraph& ref_graph) const;
  void RestoreMetaDataAfterLoading(const nsAbstractObjectGraph& graph, bool bUndoable);

  void GetMetaDataHash(const nsDocumentObject* pObject, nsUInt64& inout_uiHash) const;
  bool CopySelectedObjects(nsAbstractObjectGraph& out_objectGraph) const;
  bool PasteObjects(const nsArrayPtr<nsDocument::PasteInfo>& info, const nsAbstractObjectGraph& objectGraph, const nsVec2& vPickedPosition, bool bAllowPickedPosition);

protected:
  /// \brief Tests whether pTarget can be reached from pSource by following the pin connections
  bool CanReachNode(const nsDocumentObject* pSource, const nsDocumentObject* pTarget, nsSet<const nsDocumentObject*>& Visited) const;

  /// \brief Returns true if adding a connection between the two pins would create a circular graph
  bool WouldConnectionCreateCircle(const nsPin& source, const nsPin& target) const;

  nsResult ResolveConnection(const nsUuid& sourceObject, const nsUuid& targetObject, nsStringView sourcePin, nsStringView targetPin, const nsPin*& out_pSourcePin, const nsPin*& out_pTargetPin) const;

  virtual void GetDynamicPinNames(const nsDocumentObject* pObject, nsStringView sPropertyName, nsStringView sPinName, nsDynamicArray<nsString>& out_Names) const;
  virtual bool TryRecreatePins(const nsDocumentObject* pObject);

  struct NodeInternal
  {
    nsVec2 m_vPos = nsVec2::MakeZero();
    nsHybridArray<nsUniquePtr<nsPin>, 6> m_Inputs;
    nsHybridArray<nsUniquePtr<nsPin>, 6> m_Outputs;
  };

private:
  virtual bool InternalIsNode(const nsDocumentObject* pObject) const;
  virtual bool InternalIsConnection(const nsDocumentObject* pObject) const;
  virtual bool InternalIsDynamicPinProperty(const nsDocumentObject* pObject, const nsAbstractProperty* pProp) const { return false; }
  virtual nsStatus InternalCanConnect(const nsPin& source, const nsPin& target, CanConnectResult& out_Result) const;
  virtual nsStatus InternalCanDisconnect(const nsPin& source, const nsPin& target) const { return nsStatus(NS_SUCCESS); }
  virtual nsStatus InternalCanMoveNode(const nsDocumentObject* pObject, const nsVec2& vPos) const { return nsStatus(NS_SUCCESS); }
  virtual void InternalCreatePins(const nsDocumentObject* pObject, NodeInternal& node) = 0;

  void ObjectHandler(const nsDocumentObjectEvent& e);
  void StructureEventHandler(const nsDocumentObjectStructureEvent& e);
  void PropertyEventsHandler(const nsDocumentObjectPropertyEvent& e);

  void HandlePotentialDynamicPinPropertyChanged(const nsDocumentObject* pObject, nsStringView sPropertyName);

private:
  nsHashTable<nsUuid, NodeInternal> m_ObjectToNode;
  nsHashTable<nsUuid, nsUniquePtr<nsConnection>> m_ObjectToConnection;
  nsMap<const nsPin*, nsHybridArray<const nsConnection*, 6>> m_Connections;
};
