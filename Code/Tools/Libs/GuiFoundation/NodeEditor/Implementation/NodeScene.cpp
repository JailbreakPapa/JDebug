/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Strings/TranslationLookup.h>
#include <GuiFoundation/NodeEditor/Connection.h>
#include <GuiFoundation/NodeEditor/Node.h>
#include <GuiFoundation/NodeEditor/Pin.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <GuiFoundation/Widgets/SearchableMenu.moc.h>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <ToolsFoundation/Command/NodeCommands.h>
#include <ToolsFoundation/Command/TreeCommands.h>

nsRttiMappedObjectFactory<nsQtNode> nsQtNodeScene::s_NodeFactory;
nsRttiMappedObjectFactory<nsQtPin> nsQtNodeScene::s_PinFactory;
nsRttiMappedObjectFactory<nsQtConnection> nsQtNodeScene::s_ConnectionFactory;
nsVec2 nsQtNodeScene::s_vLastMouseInteraction(0);

nsQtNodeScene::nsQtNodeScene(QObject* pParent)
  : QGraphicsScene(pParent)
{
  setItemIndexMethod(QGraphicsScene::NoIndex);

  connect(this, &QGraphicsScene::selectionChanged, this, &nsQtNodeScene::OnSelectionChanged);
}

nsQtNodeScene::~nsQtNodeScene()
{
  disconnect(this, &QGraphicsScene::selectionChanged, this, &nsQtNodeScene::OnSelectionChanged);

  Clear();

  if (m_pManager != nullptr)
  {
    m_pManager->m_NodeEvents.RemoveEventHandler(nsMakeDelegate(&nsQtNodeScene::NodeEventsHandler, this));
    m_pManager->GetDocument()->GetSelectionManager()->m_Events.RemoveEventHandler(nsMakeDelegate(&nsQtNodeScene::SelectionEventsHandler, this));
    m_pManager->m_PropertyEvents.RemoveEventHandler(nsMakeDelegate(&nsQtNodeScene::PropertyEventsHandler, this));
  }
}

void nsQtNodeScene::InitScene(const nsDocumentNodeManager* pManager)
{
  NS_ASSERT_DEV(pManager != nullptr, "Invalid node manager");

  m_pManager = pManager;

  m_pManager->m_NodeEvents.AddEventHandler(nsMakeDelegate(&nsQtNodeScene::NodeEventsHandler, this));
  m_pManager->GetDocument()->GetSelectionManager()->m_Events.AddEventHandler(nsMakeDelegate(&nsQtNodeScene::SelectionEventsHandler, this));
  m_pManager->m_PropertyEvents.AddEventHandler(nsMakeDelegate(&nsQtNodeScene::PropertyEventsHandler, this));

  // Create Nodes
  const auto& rootObjects = pManager->GetRootObject()->GetChildren();
  for (const auto& pObject : rootObjects)
  {
    if (pManager->IsNode(pObject))
    {
      CreateQtNode(pObject);
    }
  }
  for (const auto& pObject : rootObjects)
  {
    if (pManager->IsConnection(pObject))
    {
      CreateQtConnection(pObject);
    }
  }
}

const nsDocument* nsQtNodeScene::GetDocument() const
{
  return m_pManager->GetDocument();
}

const nsDocumentNodeManager* nsQtNodeScene::GetDocumentNodeManager() const
{
  return m_pManager;
}

nsRttiMappedObjectFactory<nsQtNode>& nsQtNodeScene::GetNodeFactory()
{
  return s_NodeFactory;
}

nsRttiMappedObjectFactory<nsQtPin>& nsQtNodeScene::GetPinFactory()
{
  return s_PinFactory;
}

nsRttiMappedObjectFactory<nsQtConnection>& nsQtNodeScene::GetConnectionFactory()
{
  return s_ConnectionFactory;
}

void nsQtNodeScene::SetConnectionStyle(nsEnum<ConnectionStyle> style)
{
  m_ConnectionStyle = style;
  invalidate();
}

void nsQtNodeScene::SetConnectionDecorationFlags(nsBitflags<ConnectionDecorationFlags> flags)
{
  m_ConnectionDecorationFlags = flags;
  invalidate();
}

void nsQtNodeScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  m_vMousePos = nsVec2(event->scenePos().x(), event->scenePos().y());
  s_vLastMouseInteraction = m_vMousePos;

  if (m_pTempConnection)
  {
    event->accept();

    nsVec2 bestPos = m_vMousePos;

    // snap to the closest pin that we can connect to
    if (!m_ConnectablePins.IsEmpty())
    {
      const float fPinSize = m_ConnectablePins[0]->sceneBoundingRect().height();

      // this is also the threshold at which we snap to another position
      float fDistToBest = nsMath::Square(fPinSize * 2.5f);

      for (auto pin : m_ConnectablePins)
      {
        const QPointF center = pin->sceneBoundingRect().center();
        const nsVec2 pt = nsVec2(center.x(), center.y());
        const float lenSqr = (pt - s_vLastMouseInteraction).GetLengthSquared();

        if (lenSqr < fDistToBest)
        {
          fDistToBest = lenSqr;
          bestPos = pt;
        }
      }
    }

    if (m_pStartPin->GetPin()->GetType() == nsPin::Type::Input)
    {
      m_pTempConnection->SetPosOut(QPointF(bestPos.x, bestPos.y));
    }
    else
    {
      m_pTempConnection->SetPosIn(QPointF(bestPos.x, bestPos.y));
    }
    return;
  }

  QGraphicsScene::mouseMoveEvent(event);
}

void nsQtNodeScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  switch (event->button())
  {
    case Qt::LeftButton:
    {
      QList<QGraphicsItem*> itemList = items(event->scenePos(), Qt::IntersectsItemBoundingRect);
      for (QGraphicsItem* item : itemList)
      {
        if (item->type() != Type::Pin)
          continue;

        event->accept();
        nsQtPin* pPin = static_cast<nsQtPin*>(item);
        m_pStartPin = pPin;
        m_pTempConnection = new nsQtConnection(nullptr);
        addItem(m_pTempConnection);
        m_pTempConnection->SetPosIn(pPin->GetPinPos());
        m_pTempConnection->SetPosOut(pPin->GetPinPos());

        if (pPin->GetPin()->GetType() == nsPin::Type::Input)
        {
          m_pTempConnection->SetDirIn(pPin->GetPinDir());
          m_pTempConnection->SetDirOut(-pPin->GetPinDir());
        }
        else
        {
          m_pTempConnection->SetDirIn(-pPin->GetPinDir());
          m_pTempConnection->SetDirOut(pPin->GetPinDir());
        }

        MarkupConnectablePins(pPin);
        return;
      }
    }
    break;
    case Qt::RightButton:
    {
      event->accept();
      return;
    }

    default:
      break;
  }

  QGraphicsScene::mousePressEvent(event);
}

void nsQtNodeScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  if (m_pTempConnection && event->button() == Qt::LeftButton)
  {
    event->accept();

    const bool startWasInput = m_pStartPin->GetPin()->GetType() == nsPin::Type::Input;
    const QPointF releasePos = startWasInput ? m_pTempConnection->GetOutPos() : m_pTempConnection->GetInPos();

    QList<QGraphicsItem*> itemList = items(releasePos, Qt::IntersectsItemBoundingRect);
    for (QGraphicsItem* item : itemList)
    {
      if (item->type() != Type::Pin)
        continue;

      nsQtPin* pPin = static_cast<nsQtPin*>(item);
      if (pPin != m_pStartPin && pPin->GetPin()->GetType() != m_pStartPin->GetPin()->GetType())
      {
        const nsPin* pSourcePin = startWasInput ? pPin->GetPin() : m_pStartPin->GetPin();
        const nsPin* pTargetPin = startWasInput ? m_pStartPin->GetPin() : pPin->GetPin();
        ConnectPinsAction(*pSourcePin, *pTargetPin);
        break;
      }
    }

    delete m_pTempConnection;
    m_pTempConnection = nullptr;
    m_pStartPin = nullptr;

    ResetConnectablePinMarkup();
    return;
  }

  QGraphicsScene::mouseReleaseEvent(event);

  nsSet<const nsDocumentObject*> moved;
  for (auto it = m_Nodes.GetIterator(); it.IsValid(); ++it)
  {
    if (it.Value()->GetFlags().IsSet(nsNodeFlags::Moved))
    {
      moved.Insert(it.Key());
      it.Value()->ResetFlags();
    }
  }

  if (!moved.IsEmpty())
  {
    nsCommandHistory* history = GetDocumentNodeManager()->GetDocument()->GetCommandHistory();
    history->StartTransaction("Move Node");

    nsStatus res;
    for (auto pObject : moved)
    {
      nsMoveNodeCommand move;
      move.m_Object = pObject->GetGuid();
      auto pos = m_Nodes[pObject]->pos();
      move.m_NewPos = nsVec2(pos.x(), pos.y());
      res = history->AddCommand(move);
      if (res.m_Result.Failed())
        break;
    }

    if (res.m_Result.Failed())
      history->CancelTransaction();
    else
      history->FinishTransaction();

    nsQtUiServices::GetSingleton()->MessageBoxStatus(res, "Move node failed");
  }
}

void nsQtNodeScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* contextMenuEvent)
{
  QTransform id;

  QGraphicsItem* pItem = itemAt(contextMenuEvent->scenePos(), id);
  int iType = pItem != nullptr ? pItem->type() : -1;
  while (pItem && !(iType >= Type::Node && iType <= Type::Connection))
  {
    pItem = pItem->parentItem();
    iType = pItem != nullptr ? pItem->type() : -1;
  }

  QMenu menu;
  if (iType == Type::Pin)
  {
    nsQtPin* pPin = static_cast<nsQtPin*>(pItem);
    QAction* pAction = new QAction("Disconnect Pin", &menu);
    menu.addAction(pAction);
    connect(pAction, &QAction::triggered, this, [this, pPin](bool bChecked) { DisconnectPinsAction(pPin); });

    pPin->ExtendContextMenu(menu);
  }
  else if (iType == Type::Node)
  {
    nsQtNode* pNode = static_cast<nsQtNode*>(pItem);

    // if we clicked on an unselected item, make it the only selected item
    if (!pNode->isSelected())
    {
      clearSelection();
      pNode->setSelected(true);
    }

    // Delete Node
    {
      QAction* pAction = new QAction("Remove", &menu);
      menu.addAction(pAction);
      connect(pAction, &QAction::triggered, this, [this](bool bChecked) { RemoveSelectedNodesAction(); });
    }

    pNode->ExtendContextMenu(menu);
  }
  else if (iType == Type::Connection)
  {
    nsQtConnection* pConnection = static_cast<nsQtConnection*>(pItem);
    QAction* pAction = new QAction("Delete Connection", &menu);
    menu.addAction(pAction);
    connect(pAction, &QAction::triggered, this, [this, pConnection](bool bChecked) { DisconnectPinsAction(pConnection); });

    pConnection->ExtendContextMenu(menu);
  }
  else
  {
    OpenSearchMenu(contextMenuEvent->screenPos());
    return;
  }

  menu.exec(contextMenuEvent->screenPos());
}

void nsQtNodeScene::keyPressEvent(QKeyEvent* event)
{
  QTransform id;
  QGraphicsItem* pItem = itemAt(QPointF(m_vMousePos.x, m_vMousePos.y), id);
  if (pItem && pItem->type() == Type::Pin)
  {
    nsQtPin* pin = static_cast<nsQtPin*>(pItem);
    if (event->key() == Qt::Key_Delete)
    {
      DisconnectPinsAction(pin);
    }

    pin->keyPressEvent(event);
  }

  if (event->key() == Qt::Key_Delete)
  {
    RemoveSelectedNodesAction();
  }
  else if (event->key() == Qt::Key_Space)
  {
    OpenSearchMenu(QCursor::pos());
  }
}

void nsQtNodeScene::Clear()
{
  while (!m_Connections.IsEmpty())
  {
    DeleteQtConnection(m_Connections.GetIterator().Key());
  }

  while (!m_Nodes.IsEmpty())
  {
    DeleteQtNode(m_Nodes.GetIterator().Key());
  }
}

void nsQtNodeScene::CreateQtNode(const nsDocumentObject* pObject)
{
  nsVec2 vPos = m_pManager->GetNodePos(pObject);

  nsQtNode* pNode = s_NodeFactory.CreateObject(pObject->GetTypeAccessor().GetType());
  if (pNode == nullptr)
  {
    pNode = new nsQtNode();
  }
  m_Nodes[pObject] = pNode;
  addItem(pNode);
  pNode->InitNode(m_pManager, pObject);
  pNode->setPos(vPos.x, vPos.y);

  pNode->ResetFlags();
}

void nsQtNodeScene::DeleteQtNode(const nsDocumentObject* pObject)
{
  nsQtNode* pNode = m_Nodes[pObject];
  m_Nodes.Remove(pObject);

  removeItem(pNode);
  delete pNode;
}

void nsQtNodeScene::CreateQtConnection(const nsDocumentObject* pObject)
{
  const nsConnection& connection = m_pManager->GetConnection(pObject);
  const nsPin& pinSource = connection.GetSourcePin();
  const nsPin& pinTarget = connection.GetTargetPin();

  nsQtNode* pSource = m_Nodes[pinSource.GetParent()];
  nsQtNode* pTarget = m_Nodes[pinTarget.GetParent()];
  nsQtPin* pOutput = pSource->GetOutputPin(pinSource);
  nsQtPin* pInput = pTarget->GetInputPin(pinTarget);
  NS_ASSERT_DEV(pOutput != nullptr && pInput != nullptr, "Node does not contain pin!");

  nsQtConnection* pQtConnection = s_ConnectionFactory.CreateObject(pObject->GetTypeAccessor().GetType());
  if (pQtConnection == nullptr)
  {
    pQtConnection = new nsQtConnection(nullptr);
  }

  addItem(pQtConnection);
  pQtConnection->InitConnection(pObject, &connection);
  pOutput->AddConnection(pQtConnection);
  pInput->AddConnection(pQtConnection);
  m_Connections[pObject] = pQtConnection;

  // reset flags to update the node's title to reflect connection changes
  pSource->ResetFlags();
  pTarget->ResetFlags();
}

void nsQtNodeScene::DeleteQtConnection(const nsDocumentObject* pObject)
{
  nsQtConnection* pQtConnection = m_Connections[pObject];
  m_Connections.Remove(pObject);

  const nsConnection* pConnection = pQtConnection->GetConnection();
  NS_ASSERT_DEV(pConnection != nullptr, "No connection");

  const nsPin& pinSource = pConnection->GetSourcePin();
  const nsPin& pinTarget = pConnection->GetTargetPin();

  nsQtNode* pSource = m_Nodes[pinSource.GetParent()];
  nsQtNode* pTarget = m_Nodes[pinTarget.GetParent()];
  nsQtPin* pOutput = pSource->GetOutputPin(pinSource);
  nsQtPin* pInput = pTarget->GetInputPin(pinTarget);
  NS_ASSERT_DEV(pOutput != nullptr && pInput != nullptr, "Node does not contain pin!");

  pOutput->RemoveConnection(pQtConnection);
  pInput->RemoveConnection(pQtConnection);

  removeItem(pQtConnection);
  delete pQtConnection;

  // reset flags to update the node's title to reflect connection changes
  pSource->ResetFlags();
  pTarget->ResetFlags();
}

void nsQtNodeScene::RecreateQtPins(const nsDocumentObject* pObject)
{
  nsQtNode* pNode = m_Nodes[pObject];
  pNode->CreatePins();
  pNode->UpdateState();
  pNode->UpdateGeometry();
}

void nsQtNodeScene::CreateNodeObject(const nsRTTI* pRtti)
{
  nsCommandHistory* history = m_pManager->GetDocument()->GetCommandHistory();
  history->StartTransaction("Add Node");

  nsStatus res;
  {
    nsAddObjectCommand cmd;
    cmd.m_pType = pRtti;
    cmd.m_NewObjectGuid = nsUuid::MakeUuid();
    cmd.m_Index = -1;

    res = history->AddCommand(cmd);
    if (res.m_Result.Succeeded())
    {
      nsMoveNodeCommand move;
      move.m_Object = cmd.m_NewObjectGuid;
      move.m_NewPos = m_vMousePos;
      res = history->AddCommand(move);
    }
  }

  if (res.m_Result.Failed())
    history->CancelTransaction();
  else
    history->FinishTransaction();

  nsQtUiServices::GetSingleton()->MessageBoxStatus(res, "Adding sub-element to the property failed.");
}

void nsQtNodeScene::NodeEventsHandler(const nsDocumentNodeManagerEvent& e)
{
  switch (e.m_EventType)
  {
    case nsDocumentNodeManagerEvent::Type::NodeMoved:
    {
      nsVec2 vPos = m_pManager->GetNodePos(e.m_pObject);
      nsQtNode* pNode = m_Nodes[e.m_pObject];
      pNode->setPos(vPos.x, vPos.y);
    }
    break;
    case nsDocumentNodeManagerEvent::Type::AfterPinsConnected:
      CreateQtConnection(e.m_pObject);
      break;

    case nsDocumentNodeManagerEvent::Type::BeforePinsDisonnected:
      DeleteQtConnection(e.m_pObject);
      break;

    case nsDocumentNodeManagerEvent::Type::BeforePinsChanged:
      break;

    case nsDocumentNodeManagerEvent::Type::AfterPinsChanged:
      RecreateQtPins(e.m_pObject);
      break;

    case nsDocumentNodeManagerEvent::Type::AfterNodeAdded:
      CreateQtNode(e.m_pObject);
      break;

    case nsDocumentNodeManagerEvent::Type::BeforeNodeRemoved:
      DeleteQtNode(e.m_pObject);
      break;

    default:
      break;
  }
}

void nsQtNodeScene::PropertyEventsHandler(const nsDocumentObjectPropertyEvent& e)
{
  auto it = m_Nodes.Find(e.m_pObject);
  if (it.IsValid())
  {
    it.Value()->ResetFlags();
    it.Value()->update();
  }
}

void nsQtNodeScene::SelectionEventsHandler(const nsSelectionManagerEvent& e)
{
  const nsDeque<const nsDocumentObject*>& selection = GetDocument()->GetSelectionManager()->GetSelection();

  if (!m_bIgnoreSelectionChange)
  {
    m_bIgnoreSelectionChange = true;

    clearSelection();

    QList<QGraphicsItem*> qSelection;
    for (const nsDocumentObject* pObject : selection)
    {
      auto it = m_Nodes.Find(pObject);
      if (!it.IsValid())
        continue;

      it.Value()->setSelected(true);
    }
    m_bIgnoreSelectionChange = false;
  }

  bool bAnyPaintChanges = false;

  for (auto itCon : m_Connections)
  {
    auto pQtCon = itCon.Value();
    auto pCon = pQtCon->GetConnection();

    const bool prev = pQtCon->m_bAdjacentNodeSelected;

    pQtCon->m_bAdjacentNodeSelected = false;

    for (const nsDocumentObject* pObject : selection)
    {
      if (pCon->GetSourcePin().GetParent() == pObject || pCon->GetTargetPin().GetParent() == pObject)
      {
        pQtCon->m_bAdjacentNodeSelected = true;
        break;
      }
    }

    if (prev != pQtCon->m_bAdjacentNodeSelected)
    {
      bAnyPaintChanges = true;
    }
  }

  if (bAnyPaintChanges)
  {
    invalidate();
  }
}

void nsQtNodeScene::GetSelectedNodes(nsDeque<nsQtNode*>& selection) const
{
  selection.Clear();
  auto items = selectedItems();
  for (QGraphicsItem* pItem : items)
  {
    if (pItem->type() == nsQtNodeScene::Node)
    {
      nsQtNode* pNode = static_cast<nsQtNode*>(pItem);
      selection.PushBack(pNode);
    }
  }
}

void nsQtNodeScene::MarkupConnectablePins(nsQtPin* pQtSourcePin)
{
  m_ConnectablePins.Clear();

  const nsRTTI* pConnectionType = m_pManager->GetConnectionType();

  const nsPin* pSourcePin = pQtSourcePin->GetPin();
  const bool bConnectForward = pSourcePin->GetType() == nsPin::Type::Output;

  for (auto it = m_Nodes.GetIterator(); it.IsValid(); ++it)
  {
    const nsDocumentObject* pDocObject = it.Key();
    nsQtNode* pTargetNode = it.Value();

    {
      auto pinArray = bConnectForward ? m_pManager->GetInputPins(pDocObject) : m_pManager->GetOutputPins(pDocObject);

      for (auto& pin : pinArray)
      {
        nsQtPin* pQtTargetPin = bConnectForward ? pTargetNode->GetInputPin(*pin) : pTargetNode->GetOutputPin(*pin);

        nsDocumentNodeManager::CanConnectResult res;

        if (bConnectForward)
          m_pManager->CanConnect(pConnectionType, *pSourcePin, *pin, res).IgnoreResult();
        else
          m_pManager->CanConnect(pConnectionType, *pin, *pSourcePin, res).IgnoreResult();

        if (res == nsDocumentNodeManager::CanConnectResult::ConnectNever)
        {
          pQtTargetPin->SetHighlightState(nsQtPinHighlightState::CannotConnect);
        }
        else
        {
          m_ConnectablePins.PushBack(pQtTargetPin);

          if (res == nsDocumentNodeManager::CanConnectResult::Connect1toN || res == nsDocumentNodeManager::CanConnectResult::ConnectNtoN)
          {
            pQtTargetPin->SetHighlightState(nsQtPinHighlightState::CanAddConnection);
          }
          else
          {
            pQtTargetPin->SetHighlightState(nsQtPinHighlightState::CanReplaceConnection);
          }
        }
      }
    }

    {
      auto pinArray = !bConnectForward ? m_pManager->GetInputPins(pDocObject) : m_pManager->GetOutputPins(pDocObject);

      for (auto& pin : pinArray)
      {
        nsQtPin* pQtTargetPin = !bConnectForward ? pTargetNode->GetInputPin(*pin) : pTargetNode->GetOutputPin(*pin);
        pQtTargetPin->SetHighlightState(nsQtPinHighlightState::CannotConnectSameDirection);
      }
    }
  }
}

void nsQtNodeScene::ResetConnectablePinMarkup()
{
  m_ConnectablePins.Clear();

  for (auto it = m_Nodes.GetIterator(); it.IsValid(); ++it)
  {
    const nsDocumentObject* pDocObject = it.Key();
    nsQtNode* pTargetNode = it.Value();

    for (auto& pin : m_pManager->GetInputPins(pDocObject))
    {
      nsQtPin* pQtTargetPin = pTargetNode->GetInputPin(*pin);
      pQtTargetPin->SetHighlightState(nsQtPinHighlightState::None);
    }

    for (auto& pin : m_pManager->GetOutputPins(pDocObject))
    {
      nsQtPin* pQtTargetPin = pTargetNode->GetOutputPin(*pin);
      pQtTargetPin->SetHighlightState(nsQtPinHighlightState::None);
    }
  }
}

void nsQtNodeScene::OpenSearchMenu(QPoint screenPos)
{
  QMenu menu;
  nsQtSearchableMenu* pSearchMenu = new nsQtSearchableMenu(&menu);
  menu.addAction(pSearchMenu);

  connect(pSearchMenu, &nsQtSearchableMenu::MenuItemTriggered, this, &nsQtNodeScene::OnMenuItemTriggered);
  connect(pSearchMenu, &nsQtSearchableMenu::MenuItemTriggered, this, [&menu]() { menu.close(); });

  nsStringBuilder tmp;
  nsStringBuilder sFullPath;

  nsHybridArray<const nsRTTI*, 32> types;
  m_pManager->GetCreateableTypes(types);

  for (const nsRTTI* pRtti : types)
  {
    nsStringView sCleanName = pRtti->GetTypeName();

    if (const char* szUnderscore = sCleanName.FindLastSubString("_"))
    {
      sCleanName.SetStartPosition(szUnderscore + 1);
    }

    if (const char* szBracket = sCleanName.FindLastSubString("<"))
    {
      sCleanName = nsStringView(sCleanName.GetStartPointer(), szBracket);
    }

    sFullPath = m_pManager->GetTypeCategory(pRtti);

    if (sFullPath.IsEmpty())
    {
      if (auto pAttr = pRtti->GetAttributeByType<nsCategoryAttribute>())
      {
        sFullPath = pAttr->GetCategory();
      }
    }

    sFullPath.AppendPath(sCleanName);

    pSearchMenu->AddItem(nsTranslate(sCleanName.GetData(tmp)), sFullPath, QVariant::fromValue((void*)pRtti));
  }

  pSearchMenu->Finalize(m_sContextMenuSearchText);

  menu.exec(screenPos);

  m_sContextMenuSearchText = pSearchMenu->GetSearchText();
}

nsStatus nsQtNodeScene::RemoveNode(nsQtNode* pNode)
{
  NS_SUCCEED_OR_RETURN(m_pManager->CanRemove(pNode->GetObject()));

  nsRemoveNodeCommand cmd;
  cmd.m_Object = pNode->GetObject()->GetGuid();

  nsCommandHistory* history = GetDocumentNodeManager()->GetDocument()->GetCommandHistory();
  return history->AddCommand(cmd);
}

void nsQtNodeScene::RemoveSelectedNodesAction()
{
  nsDeque<nsQtNode*> selection;
  GetSelectedNodes(selection);

  if (selection.IsEmpty())
    return;

  nsCommandHistory* history = GetDocumentNodeManager()->GetDocument()->GetCommandHistory();
  history->StartTransaction("Remove Nodes");

  for (nsQtNode* pNode : selection)
  {
    nsStatus res = RemoveNode(pNode);

    if (res.m_Result.Failed())
    {
      history->CancelTransaction();

      nsQtUiServices::GetSingleton()->MessageBoxStatus(res, "Failed to remove node");
      return;
    }
  }

  history->FinishTransaction();
}

void nsQtNodeScene::ConnectPinsAction(const nsPin& sourcePin, const nsPin& targetPin)
{
  nsDocumentNodeManager::CanConnectResult connect;
  nsStatus res = m_pManager->CanConnect(m_pManager->GetConnectionType(), sourcePin, targetPin, connect);

  if (connect == nsDocumentNodeManager::CanConnectResult::ConnectNever)
  {
    nsQtUiServices::GetSingleton()->MessageBoxStatus(res, "Failed to connect nodes.");
    return;
  }

  nsCommandHistory* history = GetDocumentNodeManager()->GetDocument()->GetCommandHistory();
  history->StartTransaction("Connect Pins");

  // disconnect everything from the source pin
  if (connect == nsDocumentNodeManager::CanConnectResult::Connect1to1 || connect == nsDocumentNodeManager::CanConnectResult::Connect1toN)
  {
    const nsArrayPtr<const nsConnection* const> connections = m_pManager->GetConnections(sourcePin);
    for (const nsConnection* pConnection : connections)
    {
      res = nsNodeCommands::DisconnectAndRemoveCommand(history, pConnection->GetParent()->GetGuid());
      if (res.Failed())
      {
        history->CancelTransaction();
        return;
      }
    }
  }

  // disconnect everything from the target pin
  if (connect == nsDocumentNodeManager::CanConnectResult::Connect1to1 || connect == nsDocumentNodeManager::CanConnectResult::ConnectNto1)
  {
    const nsArrayPtr<const nsConnection* const> connections = m_pManager->GetConnections(targetPin);
    for (const nsConnection* pConnection : connections)
    {
      res = nsNodeCommands::DisconnectAndRemoveCommand(history, pConnection->GetParent()->GetGuid());
      if (res.Failed())
      {
        history->CancelTransaction();
        return;
      }
    }
  }

  // connect the two pins
  {
    res = nsNodeCommands::AddAndConnectCommand(history, m_pManager->GetConnectionType(), sourcePin, targetPin);
    if (res.Failed())
    {
      history->CancelTransaction();
      return;
    }
  }

  history->FinishTransaction();
}

void nsQtNodeScene::DisconnectPinsAction(nsQtConnection* pConnection)
{
  nsStatus res = m_pManager->CanDisconnect(pConnection->GetConnection());
  if (res.m_Result.Succeeded())
  {
    nsCommandHistory* history = GetDocumentNodeManager()->GetDocument()->GetCommandHistory();
    history->StartTransaction("Disconnect Pins");

    res = nsNodeCommands::DisconnectAndRemoveCommand(history, pConnection->GetConnection()->GetParent()->GetGuid());
    if (res.m_Result.Failed())
      history->CancelTransaction();
    else
      history->FinishTransaction();
  }

  nsQtUiServices::GetSingleton()->MessageBoxStatus(res, "Node disconnect failed.");
}

void nsQtNodeScene::DisconnectPinsAction(nsQtPin* pPin)
{
  nsCommandHistory* history = m_pManager->GetDocument()->GetCommandHistory();
  history->StartTransaction("Disconnect Pins");

  nsStatus res = nsStatus(NS_SUCCESS);
  for (nsQtConnection* pConnection : pPin->GetConnections())
  {
    DisconnectPinsAction(pConnection);
  }

  if (res.m_Result.Failed())
    history->CancelTransaction();
  else
    history->FinishTransaction();

  nsQtUiServices::GetSingleton()->MessageBoxStatus(res, "Adding sub-element to the property failed.");
}

void nsQtNodeScene::OnMenuItemTriggered(const QString& sName, const QVariant& variant)
{
  const nsRTTI* pRtti = static_cast<const nsRTTI*>(variant.value<void*>());

  CreateNodeObject(pRtti);
}

void nsQtNodeScene::OnSelectionChanged()
{
  nsCommandHistory* pHistory = m_pManager->GetDocument()->GetCommandHistory();
  if (pHistory->IsInUndoRedo() || pHistory->IsInTransaction())
    return;

  m_Selection.Clear();
  auto items = selectedItems();
  for (QGraphicsItem* pItem : items)
  {
    if (pItem->type() == nsQtNodeScene::Node)
    {
      nsQtNode* pNode = static_cast<nsQtNode*>(pItem);
      m_Selection.PushBack(pNode->GetObject());
    }
    else if (pItem->type() == nsQtNodeScene::Connection)
    {
      nsQtConnection* pConnection = static_cast<nsQtConnection*>(pItem);
      m_Selection.PushBack(pConnection->GetObject());
    }
  }

  if (!m_bIgnoreSelectionChange)
  {
    m_bIgnoreSelectionChange = true;
    m_pManager->GetDocument()->GetSelectionManager()->SetSelection(m_Selection);
    m_bIgnoreSelectionChange = false;
  }
}
