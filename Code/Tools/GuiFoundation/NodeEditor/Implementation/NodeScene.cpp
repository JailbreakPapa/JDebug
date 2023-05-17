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

wdRttiMappedObjectFactory<wdQtNode> wdQtNodeScene::s_NodeFactory;
wdRttiMappedObjectFactory<wdQtPin> wdQtNodeScene::s_PinFactory;
wdRttiMappedObjectFactory<wdQtConnection> wdQtNodeScene::s_ConnectionFactory;
wdVec2 wdQtNodeScene::s_vLastMouseInteraction(0);

wdQtNodeScene::wdQtNodeScene(QObject* pParent)
  : QGraphicsScene(pParent)
{
  setItemIndexMethod(QGraphicsScene::NoIndex);

  connect(this, &QGraphicsScene::selectionChanged, this, &wdQtNodeScene::OnSelectionChanged);
}

wdQtNodeScene::~wdQtNodeScene()
{
  SetDocumentNodeManager(nullptr);
}

void wdQtNodeScene::SetDocumentNodeManager(const wdDocumentNodeManager* pManager)
{
  if (pManager == m_pManager)
    return;

  Clear();
  if (m_pManager != nullptr)
  {
    m_pManager->m_NodeEvents.RemoveEventHandler(wdMakeDelegate(&wdQtNodeScene::NodeEventsHandler, this));
    m_pManager->GetDocument()->GetSelectionManager()->m_Events.RemoveEventHandler(wdMakeDelegate(&wdQtNodeScene::SelectionEventsHandler, this));
    m_pManager->m_PropertyEvents.RemoveEventHandler(wdMakeDelegate(&wdQtNodeScene::PropertyEventsHandler, this));
  }

  m_pManager = pManager;

  if (pManager != nullptr)
  {
    pManager->m_NodeEvents.AddEventHandler(wdMakeDelegate(&wdQtNodeScene::NodeEventsHandler, this));
    m_pManager->GetDocument()->GetSelectionManager()->m_Events.AddEventHandler(wdMakeDelegate(&wdQtNodeScene::SelectionEventsHandler, this));
    m_pManager->m_PropertyEvents.AddEventHandler(wdMakeDelegate(&wdQtNodeScene::PropertyEventsHandler, this));

    // Create Nodes
    const auto& rootObjects = pManager->GetRootObject()->GetChildren();
    for (const auto& pObject : rootObjects)
    {
      if (pManager->IsNode(pObject))
      {
        CreateQtNode(pObject);
      }
      else if (pManager->IsConnection(pObject))
      {
        CreateQtConnection(pObject);
      }
    }
  }
}

const wdDocument* wdQtNodeScene::GetDocument() const
{
  return m_pManager->GetDocument();
}

const wdDocumentNodeManager* wdQtNodeScene::GetDocumentNodeManager() const
{
  return m_pManager;
}

wdRttiMappedObjectFactory<wdQtNode>& wdQtNodeScene::GetNodeFactory()
{
  return s_NodeFactory;
}

wdRttiMappedObjectFactory<wdQtPin>& wdQtNodeScene::GetPinFactory()
{
  return s_PinFactory;
}

wdRttiMappedObjectFactory<wdQtConnection>& wdQtNodeScene::GetConnectionFactory()
{
  return s_ConnectionFactory;
}

void wdQtNodeScene::SetConnectionStyle(wdEnum<ConnectionStyle> style)
{
  m_ConnectionStyle = style;
  invalidate();
}

void wdQtNodeScene::SetConnectionDecorationFlags(wdBitflags<ConnectionDecorationFlags> flags)
{
  m_ConnectionDecorationFlags = flags;
  invalidate();
}

void wdQtNodeScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  m_vMousePos = wdVec2(event->scenePos().x(), event->scenePos().y());
  s_vLastMouseInteraction = m_vMousePos;

  if (m_pTempConnection)
  {
    event->accept();

    wdVec2 bestPos = m_vMousePos;

    // snap to the closest pin that we can connect to
    if (!m_ConnectablePins.IsEmpty())
    {
      const float fPinSize = m_ConnectablePins[0]->sceneBoundingRect().height();

      // this is also the threshold at which we snap to another position
      float fDistToBest = wdMath::Square(fPinSize * 2.5f);

      for (auto pin : m_ConnectablePins)
      {
        const QPointF center = pin->sceneBoundingRect().center();
        const wdVec2 pt = wdVec2(center.x(), center.y());
        const float lenSqr = (pt - s_vLastMouseInteraction).GetLengthSquared();

        if (lenSqr < fDistToBest)
        {
          fDistToBest = lenSqr;
          bestPos = pt;
        }
      }
    }

    if (m_pStartPin->GetPin()->GetType() == wdPin::Type::Input)
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

void wdQtNodeScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
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
        wdQtPin* pPin = static_cast<wdQtPin*>(item);
        m_pStartPin = pPin;
        m_pTempConnection = new wdQtConnection(nullptr);
        addItem(m_pTempConnection);
        m_pTempConnection->SetPosIn(pPin->GetPinPos());
        m_pTempConnection->SetPosOut(pPin->GetPinPos());

        if (pPin->GetPin()->GetType() == wdPin::Type::Input)
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

void wdQtNodeScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  if (m_pTempConnection && event->button() == Qt::LeftButton)
  {
    event->accept();

    const bool startWasInput = m_pStartPin->GetPin()->GetType() == wdPin::Type::Input;
    const QPointF releasePos = startWasInput ? m_pTempConnection->GetOutPos() : m_pTempConnection->GetInPos();

    QList<QGraphicsItem*> itemList = items(releasePos, Qt::IntersectsItemBoundingRect);
    for (QGraphicsItem* item : itemList)
    {
      if (item->type() != Type::Pin)
        continue;

      wdQtPin* pPin = static_cast<wdQtPin*>(item);
      if (pPin != m_pStartPin && pPin->GetPin()->GetType() != m_pStartPin->GetPin()->GetType())
      {
        const wdPin* pSourcePin = startWasInput ? pPin->GetPin() : m_pStartPin->GetPin();
        const wdPin* pTargetPin = startWasInput ? m_pStartPin->GetPin() : pPin->GetPin();
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

  wdSet<const wdDocumentObject*> moved;
  for (auto it = m_Nodes.GetIterator(); it.IsValid(); ++it)
  {
    if (it.Value()->GetFlags().IsSet(wdNodeFlags::Moved))
    {
      moved.Insert(it.Key());
    }
    it.Value()->ResetFlags();
  }

  if (!moved.IsEmpty())
  {
    wdCommandHistory* history = GetDocumentNodeManager()->GetDocument()->GetCommandHistory();
    history->StartTransaction("Move Node");

    wdStatus res;
    for (auto pObject : moved)
    {
      wdMoveNodeCommand move;
      move.m_Object = pObject->GetGuid();
      auto pos = m_Nodes[pObject]->pos();
      move.m_NewPos = wdVec2(pos.x(), pos.y());
      res = history->AddCommand(move);
      if (res.m_Result.Failed())
        break;
    }

    if (res.m_Result.Failed())
      history->CancelTransaction();
    else
      history->FinishTransaction();

    wdQtUiServices::GetSingleton()->MessageBoxStatus(res, "Move node failed");
  }
}

void wdQtNodeScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* contextMenuEvent)
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
    wdQtPin* pPin = static_cast<wdQtPin*>(pItem);
    QAction* pAction = new QAction("Disconnect Pin", &menu);
    menu.addAction(pAction);
    connect(pAction, &QAction::triggered, this, [this, pPin](bool bChecked) { DisconnectPinsAction(pPin); });

    pPin->ExtendContextMenu(menu);
  }
  else if (iType == Type::Node)
  {
    wdQtNode* pNode = static_cast<wdQtNode*>(pItem);

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
    wdQtConnection* pConnection = static_cast<wdQtConnection*>(pItem);
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

void wdQtNodeScene::keyPressEvent(QKeyEvent* event)
{
  QTransform id;
  QGraphicsItem* pItem = itemAt(QPointF(m_vMousePos.x, m_vMousePos.y), id);
  if (pItem && pItem->type() == Type::Pin)
  {
    wdQtPin* pin = static_cast<wdQtPin*>(pItem);
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

void wdQtNodeScene::Clear()
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

void wdQtNodeScene::CreateQtNode(const wdDocumentObject* pObject)
{
  wdVec2 vPos = m_pManager->GetNodePos(pObject);

  wdQtNode* pNode = s_NodeFactory.CreateObject(pObject->GetTypeAccessor().GetType());
  if (pNode == nullptr)
  {
    pNode = new wdQtNode();
  }
  m_Nodes[pObject] = pNode;
  addItem(pNode);
  pNode->InitNode(m_pManager, pObject);
  pNode->setPos(vPos.x, vPos.y);

  pNode->ResetFlags();
}

void wdQtNodeScene::DeleteQtNode(const wdDocumentObject* pObject)
{
  wdQtNode* pNode = m_Nodes[pObject];
  m_Nodes.Remove(pObject);

  removeItem(pNode);
  delete pNode;
}

void wdQtNodeScene::CreateQtConnection(const wdDocumentObject* pObject)
{
  const wdConnection& connection = m_pManager->GetConnection(pObject);
  const wdPin& pinSource = connection.GetSourcePin();
  const wdPin& pinTarget = connection.GetTargetPin();

  wdQtNode* pSource = m_Nodes[pinSource.GetParent()];
  wdQtNode* pTarget = m_Nodes[pinTarget.GetParent()];
  wdQtPin* pOutput = pSource->GetOutputPin(pinSource);
  wdQtPin* pInput = pTarget->GetInputPin(pinTarget);
  WD_ASSERT_DEV(pOutput != nullptr && pInput != nullptr, "Node does not contain pin!");

  wdQtConnection* pQtConnection = s_ConnectionFactory.CreateObject(pObject->GetTypeAccessor().GetType());
  if (pQtConnection == nullptr)
  {
    pQtConnection = new wdQtConnection(nullptr);
  }

  addItem(pQtConnection);
  pQtConnection->InitConnection(pObject, &connection);
  pOutput->AddConnection(pQtConnection);
  pInput->AddConnection(pQtConnection);
  m_Connections[pObject] = pQtConnection;
}

void wdQtNodeScene::DeleteQtConnection(const wdDocumentObject* pObject)
{
  wdQtConnection* pQtConnection = m_Connections[pObject];
  m_Connections.Remove(pObject);

  const wdConnection* pConnection = pQtConnection->GetConnection();
  WD_ASSERT_DEV(pConnection != nullptr, "No connection");

  const wdPin& pinSource = pConnection->GetSourcePin();
  const wdPin& pinTarget = pConnection->GetTargetPin();

  wdQtNode* pSource = m_Nodes[pinSource.GetParent()];
  wdQtNode* pTarget = m_Nodes[pinTarget.GetParent()];
  wdQtPin* pOutput = pSource->GetOutputPin(pinSource);
  wdQtPin* pInput = pTarget->GetInputPin(pinTarget);
  WD_ASSERT_DEV(pOutput != nullptr && pInput != nullptr, "Node does not contain pin!");

  pOutput->RemoveConnection(pQtConnection);
  pInput->RemoveConnection(pQtConnection);

  removeItem(pQtConnection);
  delete pQtConnection;
}

void wdQtNodeScene::CreateNodeObject(const wdRTTI* pRtti)
{
  wdCommandHistory* history = m_pManager->GetDocument()->GetCommandHistory();
  history->StartTransaction("Add Node");

  wdStatus res;
  {
    wdAddObjectCommand cmd;
    cmd.m_pType = pRtti;
    cmd.m_NewObjectGuid.CreateNewUuid();
    cmd.m_Index = -1;

    res = history->AddCommand(cmd);
    if (res.m_Result.Succeeded())
    {
      wdMoveNodeCommand move;
      move.m_Object = cmd.m_NewObjectGuid;
      move.m_NewPos = m_vMousePos;
      res = history->AddCommand(move);
    }
  }

  if (res.m_Result.Failed())
    history->CancelTransaction();
  else
    history->FinishTransaction();

  wdQtUiServices::GetSingleton()->MessageBoxStatus(res, "Adding sub-element to the property failed.");
}

void wdQtNodeScene::NodeEventsHandler(const wdDocumentNodeManagerEvent& e)
{
  switch (e.m_EventType)
  {
    case wdDocumentNodeManagerEvent::Type::NodeMoved:
    {
      wdVec2 vPos = m_pManager->GetNodePos(e.m_pObject);
      wdQtNode* pNode = m_Nodes[e.m_pObject];
      pNode->setPos(vPos.x, vPos.y);
    }
    break;
    case wdDocumentNodeManagerEvent::Type::AfterPinsConnected:
      CreateQtConnection(e.m_pObject);
      break;

    case wdDocumentNodeManagerEvent::Type::BeforePinsDisonnected:
      DeleteQtConnection(e.m_pObject);
      break;

    case wdDocumentNodeManagerEvent::Type::AfterNodeAdded:
      CreateQtNode(e.m_pObject);
      break;

    case wdDocumentNodeManagerEvent::Type::BeforeNodeRemoved:
      DeleteQtNode(e.m_pObject);
      break;

    default:
      break;
  }
}

void wdQtNodeScene::PropertyEventsHandler(const wdDocumentObjectPropertyEvent& e)
{
  auto it = m_Nodes.Find(e.m_pObject);

  if (!it.IsValid())
    return;

  it.Value()->UpdateState();
}

void wdQtNodeScene::SelectionEventsHandler(const wdSelectionManagerEvent& e)
{
  const wdDeque<const wdDocumentObject*>& selection = GetDocument()->GetSelectionManager()->GetSelection();

  if (!m_bIgnoreSelectionChange)
  {
    m_bIgnoreSelectionChange = true;

    clearSelection();

    QList<QGraphicsItem*> qSelection;
    for (const wdDocumentObject* pObject : selection)
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

    for (const wdDocumentObject* pObject : selection)
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

void wdQtNodeScene::GetSelectedNodes(wdDeque<wdQtNode*>& selection) const
{
  selection.Clear();
  auto items = selectedItems();
  for (QGraphicsItem* pItem : items)
  {
    if (pItem->type() == wdQtNodeScene::Node)
    {
      wdQtNode* pNode = static_cast<wdQtNode*>(pItem);
      selection.PushBack(pNode);
    }
  }
}

void wdQtNodeScene::MarkupConnectablePins(wdQtPin* pQtSourcePin)
{
  m_ConnectablePins.Clear();

  const wdRTTI* pConnectionType = m_pManager->GetConnectionType();

  const wdPin* pSourcePin = pQtSourcePin->GetPin();
  const bool bConnectForward = pSourcePin->GetType() == wdPin::Type::Output;

  for (auto it = m_Nodes.GetIterator(); it.IsValid(); ++it)
  {
    const wdDocumentObject* pDocObject = it.Key();
    wdQtNode* pTargetNode = it.Value();

    {
      auto pinArray = bConnectForward ? m_pManager->GetInputPins(pDocObject) : m_pManager->GetOutputPins(pDocObject);

      for (auto& pin : pinArray)
      {
        wdQtPin* pQtTargetPin = bConnectForward ? pTargetNode->GetInputPin(*pin) : pTargetNode->GetOutputPin(*pin);

        wdDocumentNodeManager::CanConnectResult res;

        if (bConnectForward)
          m_pManager->CanConnect(pConnectionType, *pSourcePin, *pin, res);
        else
          m_pManager->CanConnect(pConnectionType, *pin, *pSourcePin, res);

        if (res == wdDocumentNodeManager::CanConnectResult::ConnectNever)
        {
          pQtTargetPin->SetHighlightState(wdQtPinHighlightState::CannotConnect);
        }
        else
        {
          m_ConnectablePins.PushBack(pQtTargetPin);

          if (res == wdDocumentNodeManager::CanConnectResult::Connect1toN || res == wdDocumentNodeManager::CanConnectResult::ConnectNtoN)
          {
            pQtTargetPin->SetHighlightState(wdQtPinHighlightState::CanAddConnection);
          }
          else
          {
            pQtTargetPin->SetHighlightState(wdQtPinHighlightState::CanReplaceConnection);
          }
        }
      }
    }

    {
      auto pinArray = !bConnectForward ? m_pManager->GetInputPins(pDocObject) : m_pManager->GetOutputPins(pDocObject);

      for (auto& pin : pinArray)
      {
        wdQtPin* pQtTargetPin = !bConnectForward ? pTargetNode->GetInputPin(*pin) : pTargetNode->GetOutputPin(*pin);
        pQtTargetPin->SetHighlightState(wdQtPinHighlightState::CannotConnectSameDirection);
      }
    }
  }
}

void wdQtNodeScene::ResetConnectablePinMarkup()
{
  m_ConnectablePins.Clear();

  for (auto it = m_Nodes.GetIterator(); it.IsValid(); ++it)
  {
    const wdDocumentObject* pDocObject = it.Key();
    wdQtNode* pTargetNode = it.Value();

    for (auto& pin : m_pManager->GetInputPins(pDocObject))
    {
      wdQtPin* pQtTargetPin = pTargetNode->GetInputPin(*pin);
      pQtTargetPin->SetHighlightState(wdQtPinHighlightState::None);
    }

    for (auto& pin : m_pManager->GetOutputPins(pDocObject))
    {
      wdQtPin* pQtTargetPin = pTargetNode->GetOutputPin(*pin);
      pQtTargetPin->SetHighlightState(wdQtPinHighlightState::None);
    }
  }
}

void wdQtNodeScene::OpenSearchMenu(QPoint screenPos)
{
  QMenu menu;
  wdQtSearchableMenu* pSearchMenu = new wdQtSearchableMenu(&menu);
  menu.addAction(pSearchMenu);

  connect(pSearchMenu, &wdQtSearchableMenu::MenuItemTriggered, this, &wdQtNodeScene::OnMenuItemTriggered);
  connect(pSearchMenu, &wdQtSearchableMenu::MenuItemTriggered, this, [&menu]() { menu.close(); });

  wdStringBuilder sFullName, sCleanName;

  wdHybridArray<const wdRTTI*, 32> types;
  m_pManager->GetCreateableTypes(types);

  for (const wdRTTI* pRtti : types)
  {
    const char* szCleanName = pRtti->GetTypeName();

    const char* szColonColon = wdStringUtils::FindLastSubString(szCleanName, "::");
    if (szColonColon != nullptr)
      szCleanName = szColonColon + 2;

    const char* szUnderscore = wdStringUtils::FindLastSubString(szCleanName, "_");
    if (szUnderscore != nullptr)
      szCleanName = szUnderscore + 1;

    sCleanName = szCleanName;
    if (const char* szBracket = sCleanName.FindLastSubString("<"))
    {
      sCleanName.SetSubString_FromTo(sCleanName.GetData(), szBracket);
    }

    sFullName = m_pManager->GetTypeCategory(pRtti);

    if (sFullName.IsEmpty())
    {
      if (auto pAttr = pRtti->GetAttributeByType<wdCategoryAttribute>())
      {
        sFullName = pAttr->GetCategory();
      }
    }

    sFullName.AppendPath(wdTranslate(sCleanName));

    pSearchMenu->AddItem(sFullName, QVariant::fromValue((void*)pRtti));
  }

  pSearchMenu->Finalize(m_sContextMenuSearchText);

  menu.exec(screenPos);

  m_sContextMenuSearchText = pSearchMenu->GetSearchText();
}

wdStatus wdQtNodeScene::RemoveNode(wdQtNode* pNode)
{
  WD_SUCCEED_OR_RETURN(m_pManager->CanRemove(pNode->GetObject()));

  wdRemoveNodeCommand cmd;
  cmd.m_Object = pNode->GetObject()->GetGuid();

  wdCommandHistory* history = GetDocumentNodeManager()->GetDocument()->GetCommandHistory();
  return history->AddCommand(cmd);
}

void wdQtNodeScene::RemoveSelectedNodesAction()
{
  wdDeque<wdQtNode*> selection;
  GetSelectedNodes(selection);

  if (selection.IsEmpty())
    return;

  wdCommandHistory* history = GetDocumentNodeManager()->GetDocument()->GetCommandHistory();
  history->StartTransaction("Remove Nodes");

  for (wdQtNode* pNode : selection)
  {
    wdStatus res = RemoveNode(pNode);

    if (res.m_Result.Failed())
    {
      history->CancelTransaction();

      wdQtUiServices::GetSingleton()->MessageBoxStatus(res, "Failed to remove node");
      return;
    }
  }

  history->FinishTransaction();
}

void wdQtNodeScene::ConnectPinsAction(const wdPin& sourcePin, const wdPin& targetPin)
{
  wdDocumentNodeManager::CanConnectResult connect;
  wdStatus res = m_pManager->CanConnect(m_pManager->GetConnectionType(), sourcePin, targetPin, connect);

  if (connect == wdDocumentNodeManager::CanConnectResult::ConnectNever)
  {
    wdQtUiServices::GetSingleton()->MessageBoxStatus(res, "Failed to connect nodes.");
    return;
  }

  wdCommandHistory* history = GetDocumentNodeManager()->GetDocument()->GetCommandHistory();
  history->StartTransaction("Connect Pins");

  // disconnect everything from the source pin
  if (connect == wdDocumentNodeManager::CanConnectResult::Connect1to1 || connect == wdDocumentNodeManager::CanConnectResult::Connect1toN)
  {
    const wdArrayPtr<const wdConnection* const> connections = m_pManager->GetConnections(sourcePin);
    for (const wdConnection* pConnection : connections)
    {
      res = wdNodeCommands::DisconnectAndRemoveCommand(history, pConnection->GetParent()->GetGuid());
      if (res.Failed())
      {
        history->CancelTransaction();
        return;
      }
    }
  }

  // disconnect everything from the target pin
  if (connect == wdDocumentNodeManager::CanConnectResult::Connect1to1 || connect == wdDocumentNodeManager::CanConnectResult::ConnectNto1)
  {
    const wdArrayPtr<const wdConnection* const> connections = m_pManager->GetConnections(targetPin);
    for (const wdConnection* pConnection : connections)
    {
      res = wdNodeCommands::DisconnectAndRemoveCommand(history, pConnection->GetParent()->GetGuid());
      if (res.Failed())
      {
        history->CancelTransaction();
        return;
      }
    }
  }

  // connect the two pins
  {
    res = wdNodeCommands::AddAndConnectCommand(history, m_pManager->GetConnectionType(), sourcePin, targetPin);
    if (res.Failed())
    {
      history->CancelTransaction();
      return;
    }
  }

  history->FinishTransaction();
}

void wdQtNodeScene::DisconnectPinsAction(wdQtConnection* pConnection)
{
  wdStatus res = m_pManager->CanDisconnect(pConnection->GetConnection());
  if (res.m_Result.Succeeded())
  {
    wdCommandHistory* history = GetDocumentNodeManager()->GetDocument()->GetCommandHistory();
    history->StartTransaction("Disconnect Pins");

    res = wdNodeCommands::DisconnectAndRemoveCommand(history, pConnection->GetConnection()->GetParent()->GetGuid());
    if (res.m_Result.Failed())
      history->CancelTransaction();
    else
      history->FinishTransaction();
  }

  wdQtUiServices::GetSingleton()->MessageBoxStatus(res, "Node disconnect failed.");
}

void wdQtNodeScene::DisconnectPinsAction(wdQtPin* pPin)
{
  wdCommandHistory* history = m_pManager->GetDocument()->GetCommandHistory();
  history->StartTransaction("Disconnect Pins");

  wdStatus res = wdStatus(WD_SUCCESS);
  for (wdQtConnection* pConnection : pPin->GetConnections())
  {
    DisconnectPinsAction(pConnection);
  }

  if (res.m_Result.Failed())
    history->CancelTransaction();
  else
    history->FinishTransaction();

  wdQtUiServices::GetSingleton()->MessageBoxStatus(res, "Adding sub-element to the property failed.");
}

void wdQtNodeScene::OnMenuItemTriggered(const QString& sName, const QVariant& variant)
{
  const wdRTTI* pRtti = static_cast<const wdRTTI*>(variant.value<void*>());

  CreateNodeObject(pRtti);
}

void wdQtNodeScene::OnSelectionChanged()
{
  wdCommandHistory* pHistory = m_pManager->GetDocument()->GetCommandHistory();
  if (pHistory->IsInUndoRedo() || pHistory->IsInTransaction())
    return;

  m_Selection.Clear();
  auto items = selectedItems();
  for (QGraphicsItem* pItem : items)
  {
    if (pItem->type() == wdQtNodeScene::Node)
    {
      wdQtNode* pNode = static_cast<wdQtNode*>(pItem);
      m_Selection.PushBack(pNode->GetObject());
    }
    else if (pItem->type() == wdQtNodeScene::Connection)
    {
      wdQtConnection* pConnection = static_cast<wdQtConnection*>(pItem);
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
