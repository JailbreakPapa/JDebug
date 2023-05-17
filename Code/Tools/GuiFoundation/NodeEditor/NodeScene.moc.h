#pragma once

#include <Foundation/Containers/Map.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <ToolsFoundation/Factory/RttiMappedObjectFactory.h>
#include <ToolsFoundation/NodeObject/DocumentNodeManager.h>

class wdQtNode;
class wdQtPin;
class wdQtConnection;
struct wdSelectionManagerEvent;

class WD_GUIFOUNDATION_DLL wdQtNodeScene : public QGraphicsScene
{
  Q_OBJECT
public:
  enum Type
  {
    Node = QGraphicsItem::UserType + 1,
    Pin,
    Connection
  };

  explicit wdQtNodeScene(QObject* pParent = nullptr);
  ~wdQtNodeScene();

  void SetDocumentNodeManager(const wdDocumentNodeManager* pManager);
  const wdDocumentNodeManager* GetDocumentNodeManager() const;
  const wdDocument* GetDocument() const;

  static wdRttiMappedObjectFactory<wdQtNode>& GetNodeFactory();
  static wdRttiMappedObjectFactory<wdQtPin>& GetPinFactory();
  static wdRttiMappedObjectFactory<wdQtConnection>& GetConnectionFactory();
  static wdVec2 GetLastMouseInteractionPos() { return s_vLastMouseInteraction; }

  struct ConnectionStyle
  {
    using StorageType = wdUInt32;

    enum Enum
    {
      BwdierCurve,
      StraightLine,

      Default = BwdierCurve
    };
  };

  void SetConnectionStyle(wdEnum<ConnectionStyle> style);
  wdEnum<ConnectionStyle> GetConnectionStyle() const { return m_ConnectionStyle; }

  struct ConnectionDecorationFlags
  {
    using StorageType = wdUInt32;

    enum Enum
    {
      DirectionArrows = WD_BIT(0), ///< Draw an arrow to indicate the connection's direction. Only works with straight lines atm.

      Default = 0
    };

    struct Bits
    {
      StorageType DirectionArrows : 1;
    };
  };

  void SetConnectionDecorationFlags(wdBitflags<ConnectionDecorationFlags> flags);
  wdBitflags<ConnectionDecorationFlags> GetConnectionDecorationFlags() const { return m_ConnectionDecorationFlags; }

protected:
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
  virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
  virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* contextMenuEvent) override;
  virtual void keyPressEvent(QKeyEvent* event) override;

private:
  void Clear();
  void CreateQtNode(const wdDocumentObject* pObject);
  void DeleteQtNode(const wdDocumentObject* pObject);
  void CreateQtConnection(const wdDocumentObject* pObject);
  void DeleteQtConnection(const wdDocumentObject* pObject);
  void CreateNodeObject(const wdRTTI* pRtti);
  void NodeEventsHandler(const wdDocumentNodeManagerEvent& e);
  void PropertyEventsHandler(const wdDocumentObjectPropertyEvent& e);
  void SelectionEventsHandler(const wdSelectionManagerEvent& e);
  void GetSelectedNodes(wdDeque<wdQtNode*>& selection) const;
  void MarkupConnectablePins(wdQtPin* pSourcePin);
  void ResetConnectablePinMarkup();
  void OpenSearchMenu(QPoint screenPos);

protected:
  virtual wdStatus RemoveNode(wdQtNode* pNode);
  virtual void RemoveSelectedNodesAction();
  virtual void ConnectPinsAction(const wdPin& sourcePin, const wdPin& targetPin);
  virtual void DisconnectPinsAction(wdQtConnection* pConnection);
  virtual void DisconnectPinsAction(wdQtPin* pPin);

private Q_SLOTS:
  void OnMenuItemTriggered(const QString& sName, const QVariant& variant);
  void OnSelectionChanged();

private:
  static wdRttiMappedObjectFactory<wdQtNode> s_NodeFactory;
  static wdRttiMappedObjectFactory<wdQtPin> s_PinFactory;
  static wdRttiMappedObjectFactory<wdQtConnection> s_ConnectionFactory;

protected:
  const wdDocumentNodeManager* m_pManager = nullptr;

  wdMap<const wdDocumentObject*, wdQtNode*> m_Nodes;
  wdMap<const wdDocumentObject*, wdQtConnection*> m_Connections;

private:
  bool m_bIgnoreSelectionChange = false;
  wdQtPin* m_pStartPin = nullptr;
  wdQtConnection* m_pTempConnection = nullptr;
  wdDeque<const wdDocumentObject*> m_Selection;
  wdVec2 m_vMousePos = wdVec2::ZeroVector();
  QString m_sContextMenuSearchText;
  wdDynamicArray<const wdQtPin*> m_ConnectablePins;
  wdEnum<ConnectionStyle> m_ConnectionStyle;
  wdBitflags<ConnectionDecorationFlags> m_ConnectionDecorationFlags;

  static wdVec2 s_vLastMouseInteraction;
};

WD_DECLARE_FLAGS_OPERATORS(wdQtNodeScene::ConnectionDecorationFlags);
