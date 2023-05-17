#include <GuiFoundation/GuiFoundationPCH.h>

#include "Foundation/Strings/TranslationLookup.h"
#include <GuiFoundation/NodeEditor/Node.h>
#include <GuiFoundation/NodeEditor/Pin.h>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <ToolsFoundation/Document/Document.h>

wdQtNode::wdQtNode()
{
  auto palette = QApplication::palette();

  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIsSelectable);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);

  setBrush(palette.window());
  QPen pen(palette.light().color(), 3, Qt::SolidLine);
  setPen(pen);

  m_pLabel = new QGraphicsTextItem(this);
  m_pLabel->setDefaultTextColor(palette.buttonText().color());
  QFont font = QApplication::font();
  font.setBold(true);
  m_pLabel->setFont(font);

  m_HeaderColor = palette.alternateBase().color();
}

wdQtNode::~wdQtNode()
{
  EnableDropShadow(false);
}

void wdQtNode::EnableDropShadow(bool bEnable)
{
  if (bEnable && m_pShadow == nullptr)
  {
    auto palette = QApplication::palette();

    m_pShadow = new QGraphicsDropShadowEffect();
    m_pShadow->setOffset(3, 3);
    m_pShadow->setColor(palette.color(QPalette::Shadow));
    m_pShadow->setBlurRadius(10);
    setGraphicsEffect(m_pShadow);
  }

  if (!bEnable && m_pShadow != nullptr)
  {
    delete m_pShadow;
    m_pShadow = nullptr;
  }
}

void wdQtNode::InitNode(const wdDocumentNodeManager* pManager, const wdDocumentObject* pObject)
{
  m_pManager = pManager;
  m_pObject = pObject;
  CreatePins();
  UpdateState();

  UpdateGeometry();

  if (const wdColorAttribute* pColorAttr = pObject->GetType()->GetAttributeByType<wdColorAttribute>())
  {
    m_HeaderColor = wdToQtColor(pColorAttr->GetColor());
  }

  m_DirtyFlags.Add(wdNodeFlags::UpdateTitle);
}

void wdQtNode::UpdateGeometry()
{
  prepareGeometryChange();

  auto labelRect = m_pLabel->boundingRect();

  QFontMetrics fm(scene()->font());
  const int headerWidth = labelRect.width();
  int h = labelRect.height() + 5;

  int y = h;

  // Align inputs
  int maxInputWidth = 0;
  for (wdQtPin* pQtPin : m_Inputs)
  {
    auto rectPin = pQtPin->GetPinRect();
    pQtPin->setPos(QPointF(-rectPin.x(), y - rectPin.y()));

    maxInputWidth = wdMath::Max(maxInputWidth, (int)rectPin.width());
    y += rectPin.height();
  }

  int maxheight = y;
  y = h;

  // Align outputs
  int maxOutputWidth = 0;
  for (wdQtPin* pQtPin : m_Outputs)
  {
    auto rectPin = pQtPin->GetPinRect();
    pQtPin->setPos(QPointF(-rectPin.x(), y - rectPin.y()));

    maxOutputWidth = wdMath::Max(maxOutputWidth, (int)rectPin.width());
    y += rectPin.height();
  }

  int w = 0;

  if (maxInputWidth == 0)
    w = maxOutputWidth;
  else if (maxOutputWidth == 0)
    w = maxInputWidth;
  else
    w = wdMath::Max(maxInputWidth, maxOutputWidth) * 2;

  w += 10;
  w = wdMath::Max(w, headerWidth);


  maxheight = wdMath::Max(maxheight, y);

  // Align outputs to the right
  for (wdUInt32 i = 0; i < m_Outputs.GetCount(); ++i)
  {
    auto rectPin = m_Outputs[i]->GetPinRect();
    m_Outputs[i]->setX(w - rectPin.width());
  }

  m_HeaderRect = QRectF(-5, -5, w + 10, labelRect.height() + 10);

  {
    QPainterPath p;
    p.addRoundedRect(-5, -5, w + 10, maxheight + 10, 5, 5);
    setPath(p);
  }
}

void wdQtNode::UpdateState()
{
  auto& typeAccessor = m_pObject->GetTypeAccessor();

  wdVariant name = typeAccessor.GetValue("Name");
  if (name.IsA<wdString>() && name.Get<wdString>().IsEmpty() == false)
  {
    m_pLabel->setPlainText(name.Get<wdString>().GetData());
  }
  else
  {
    m_pLabel->setPlainText(wdTranslate(typeAccessor.GetType()->GetTypeName()));
  }
}

void wdQtNode::SetActive(bool bActive)
{
  if (m_bIsActive != bActive)
  {
    m_bIsActive = bActive;

    for (auto pInputPin : m_Inputs)
    {
      pInputPin->SetActive(bActive);
    }

    for (auto pOutputPin : m_Outputs)
    {
      pOutputPin->SetActive(bActive);
    }
  }

  update();
}

void wdQtNode::CreatePins()
{
  auto inputs = m_pManager->GetInputPins(m_pObject);
  for (auto& pPinTarget : inputs)
  {
    wdQtPin* pQtPin = wdQtNodeScene::GetPinFactory().CreateObject(pPinTarget->GetDynamicRTTI());
    if (pQtPin == nullptr)
    {
      pQtPin = new wdQtPin();
    }
    pQtPin->setParentItem(this);
    m_Inputs.PushBack(pQtPin);

    pQtPin->SetPin(*pPinTarget);
  }

  auto outputs = m_pManager->GetOutputPins(m_pObject);
  for (auto& pPinSource : outputs)
  {
    wdQtPin* pQtPin = wdQtNodeScene::GetPinFactory().CreateObject(pPinSource->GetDynamicRTTI());
    if (pQtPin == nullptr)
    {
      pQtPin = new wdQtPin();
    }

    pQtPin->setParentItem(this);
    m_Outputs.PushBack(pQtPin);

    pQtPin->SetPin(*pPinSource);
  }
}

wdQtPin* wdQtNode::GetInputPin(const wdPin& pin)
{
  for (wdQtPin* pQtPin : m_Inputs)
  {
    if (pQtPin->GetPin() == &pin)
      return pQtPin;
  }
  return nullptr;
}

wdQtPin* wdQtNode::GetOutputPin(const wdPin& pin)
{
  for (wdQtPin* pQtPin : m_Outputs)
  {
    if (pQtPin->GetPin() == &pin)
      return pQtPin;
  }
  return nullptr;
}

wdBitflags<wdNodeFlags> wdQtNode::GetFlags() const
{
  return m_DirtyFlags;
}

void wdQtNode::ResetFlags()
{
  m_DirtyFlags = wdNodeFlags::UpdateTitle;
}

void wdQtNode::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  if (m_DirtyFlags.IsSet(wdNodeFlags::UpdateTitle))
  {
    UpdateState();
    UpdateGeometry();
    m_DirtyFlags.Remove(wdNodeFlags::UpdateTitle);
  }

  auto palette = QApplication::palette();

  // Draw background
  painter->setPen(QPen(Qt::NoPen));
  painter->setBrush(brush());
  painter->drawPath(path());

  QColor headerColor = m_HeaderColor;
  if (!m_bIsActive)
    headerColor.setAlpha(50);

  // Draw header
  painter->setClipPath(path());
  painter->setPen(QPen(Qt::NoPen));
  painter->setBrush(headerColor);
  painter->drawRect(m_HeaderRect);
  painter->setClipping(false);

  QColor labelColor;

  // Draw outline
  if (isSelected())
  {
    QPen p = pen();
    p.setColor(palette.highlight().color());
    painter->setPen(p);

    labelColor = palette.highlightedText().color();
  }
  else
  {
    painter->setPen(pen());

    labelColor = palette.buttonText().color();
  }

  // Label
  if (!m_bIsActive)
    labelColor = labelColor.darker(150);

  const bool bBackgroundIsLight = m_HeaderColor.lightnessF() > 0.6f;
  if (bBackgroundIsLight)
  {
    labelColor.setRed(255 - labelColor.red());
    labelColor.setGreen(255 - labelColor.green());
    labelColor.setBlue(255 - labelColor.blue());
  }

  m_pLabel->setDefaultTextColor(labelColor);

  painter->setBrush(QBrush(Qt::NoBrush));
  painter->drawPath(path());
}

QVariant wdQtNode::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (!m_pObject)
    return QGraphicsPathItem::itemChange(change, value);

  wdCommandHistory* pHistory = m_pManager->GetDocument()->GetCommandHistory();
  switch (change)
  {
    case QGraphicsItem::ItemPositionHasChanged:
    {
      if (!pHistory->IsInUndoRedo() && !pHistory->IsInTransaction())
        m_DirtyFlags.Add(wdNodeFlags::Moved);
    }
    break;

    default:
      break;
  }
  return QGraphicsPathItem::itemChange(change, value);
}
