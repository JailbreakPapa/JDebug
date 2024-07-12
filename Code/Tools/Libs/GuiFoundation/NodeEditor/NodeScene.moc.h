/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <Foundation/Containers/Map.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <ToolsFoundation/Factory/RttiMappedObjectFactory.h>
#include <ToolsFoundation/NodeObject/DocumentNodeManager.h>

class nsQtNode;
class nsQtPin;
class nsQtConnection;
struct nsSelectionManagerEvent;

class NS_GUIFOUNDATION_DLL nsQtNodeScene : public QGraphicsScene
{
  Q_OBJECT
public:
  enum Type
  {
    Node = QGraphicsItem::UserType + 1,
    Pin,
    Connection
  };

  explicit nsQtNodeScene(QObject* pParent = nullptr);
  ~nsQtNodeScene();

  virtual void InitScene(const nsDocumentNodeManager* pManager);
  const nsDocumentNodeManager* GetDocumentNodeManager() const;
  const nsDocument* GetDocument() const;

  static nsRttiMappedObjectFactory<nsQtNode>& GetNodeFactory();
  static nsRttiMappedObjectFactory<nsQtPin>& GetPinFactory();
  static nsRttiMappedObjectFactory<nsQtConnection>& GetConnectionFactory();
  static nsVec2 GetLastMouseInteractionPos() { return s_vLastMouseInteraction; }

  struct ConnectionStyle
  {
    using StorageType = nsUInt32;

    enum Enum
    {
      BezierCurve,
      StraightLine,

      Default = BezierCurve
    };
  };

  void SetConnectionStyle(nsEnum<ConnectionStyle> style);
  nsEnum<ConnectionStyle> GetConnectionStyle() const { return m_ConnectionStyle; }

  struct ConnectionDecorationFlags
  {
    using StorageType = nsUInt32;

    enum Enum
    {
      DirectionArrows = NS_BIT(0), ///< Draw an arrow to indicate the connection's direction. Only works with straight lines atm.

      Default = 0
    };

    struct Bits
    {
      StorageType DirectionArrows : 1;
    };
  };

  void SetConnectionDecorationFlags(nsBitflags<ConnectionDecorationFlags> flags);
  nsBitflags<ConnectionDecorationFlags> GetConnectionDecorationFlags() const { return m_ConnectionDecorationFlags; }

protected:
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
  virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
  virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* contextMenuEvent) override;
  virtual void keyPressEvent(QKeyEvent* event) override;

private:
  void Clear();
  void CreateQtNode(const nsDocumentObject* pObject);
  void DeleteQtNode(const nsDocumentObject* pObject);
  void CreateQtConnection(const nsDocumentObject* pObject);
  void DeleteQtConnection(const nsDocumentObject* pObject);
  void RecreateQtPins(const nsDocumentObject* pObject);
  void CreateNodeObject(const nsRTTI* pRtti);
  void NodeEventsHandler(const nsDocumentNodeManagerEvent& e);
  void PropertyEventsHandler(const nsDocumentObjectPropertyEvent& e);
  void SelectionEventsHandler(const nsSelectionManagerEvent& e);
  void GetSelectedNodes(nsDeque<nsQtNode*>& selection) const;
  void MarkupConnectablePins(nsQtPin* pSourcePin);
  void ResetConnectablePinMarkup();
  void OpenSearchMenu(QPoint screenPos);

protected:
  virtual nsStatus RemoveNode(nsQtNode* pNode);
  virtual void RemoveSelectedNodesAction();
  virtual void ConnectPinsAction(const nsPin& sourcePin, const nsPin& targetPin);
  virtual void DisconnectPinsAction(nsQtConnection* pConnection);
  virtual void DisconnectPinsAction(nsQtPin* pPin);

private Q_SLOTS:
  void OnMenuItemTriggered(const QString& sName, const QVariant& variant);
  void OnSelectionChanged();

private:
  static nsRttiMappedObjectFactory<nsQtNode> s_NodeFactory;
  static nsRttiMappedObjectFactory<nsQtPin> s_PinFactory;
  static nsRttiMappedObjectFactory<nsQtConnection> s_ConnectionFactory;

protected:
  const nsDocumentNodeManager* m_pManager = nullptr;

  nsMap<const nsDocumentObject*, nsQtNode*> m_Nodes;
  nsMap<const nsDocumentObject*, nsQtConnection*> m_Connections;

private:
  bool m_bIgnoreSelectionChange = false;
  nsQtPin* m_pStartPin = nullptr;
  nsQtConnection* m_pTempConnection = nullptr;
  nsDeque<const nsDocumentObject*> m_Selection;
  nsVec2 m_vMousePos = nsVec2::MakeZero();
  QString m_sContextMenuSearchText;
  nsDynamicArray<const nsQtPin*> m_ConnectablePins;
  nsEnum<ConnectionStyle> m_ConnectionStyle;
  nsBitflags<ConnectionDecorationFlags> m_ConnectionDecorationFlags;

  static nsVec2 s_vLastMouseInteraction;
};

NS_DECLARE_FLAGS_OPERATORS(nsQtNodeScene::ConnectionDecorationFlags);
