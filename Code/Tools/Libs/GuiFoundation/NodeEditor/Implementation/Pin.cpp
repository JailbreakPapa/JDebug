#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Strings/TranslationLookup.h>
#include <GuiFoundation/NodeEditor/Connection.h>
#include <GuiFoundation/NodeEditor/Pin.h>
#include <QApplication>
#include <QPalette>

nsQtPin::nsQtPin()
{
  auto palette = QApplication::palette();

  QPen pen(palette.light().color(), 3, Qt::SolidLine);
  setPen(pen);
  setBrush(palette.base());

  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  setFlag(QGraphicsItem::ItemSendsScenePositionChanges);

  m_pLabel = new QGraphicsTextItem(this);
}

nsQtPin::~nsQtPin() = default;

void nsQtPin::AddConnection(nsQtConnection* pConnection)
{
  NS_ASSERT_DEBUG(!m_Connections.Contains(pConnection), "Connection already present!");
  m_Connections.PushBack(pConnection);

  ConnectedStateChanged(true);

  UpdateConnections();
}

void nsQtPin::RemoveConnection(nsQtConnection* pConnection)
{
  NS_ASSERT_DEBUG(m_Connections.Contains(pConnection), "Connection not present!");
  m_Connections.RemoveAndSwap(pConnection);

  if (m_Connections.IsEmpty())
    ConnectedStateChanged(false);
}

void nsQtPin::ConnectedStateChanged(bool bConnected)
{
  UpdatePinColors();
}

void nsQtPin::SetPin(const nsPin& pin)
{
  m_pPin = &pin;

  if (m_bTranslatePinName)
  {
    m_pLabel->setPlainText(nsMakeQString(nsTranslate(pin.GetName())));
  }
  else
  {
    m_pLabel->setPlainText(pin.GetName());
  }

  auto rectLabel = m_pLabel->boundingRect();

  const int iRadus = rectLabel.height();
  QRectF bounds;

  if (pin.GetType() == nsPin::Type::Input)
  {
    m_pLabel->setPos(iRadus, 0);
    bounds = QRectF(0, 0, iRadus, iRadus);
  }
  else
  {
    m_pLabel->setPos(0, 0);
    bounds = QRectF(rectLabel.width(), 0, iRadus, iRadus);
  }

  const int shrink = 3;
  bounds.adjust(shrink, shrink, -shrink, -shrink);
  m_PinCenter = bounds.center();

  {
    QPainterPath p;
    switch (m_pPin->m_Shape)
    {
      case nsPin::Shape::Circle:
        p.addEllipse(bounds);
        break;
      case nsPin::Shape::Rect:
        p.addRect(bounds);
        break;
      case nsPin::Shape::RoundRect:
        p.addRoundedRect(bounds, 2, 2);
        break;
      case nsPin::Shape::Arrow:
      {
        QPolygonF arrow;
        arrow.append(bounds.topLeft());
        arrow.append(QPointF(bounds.center().x(), bounds.top()));
        arrow.append(QPointF(bounds.right(), bounds.center().y()));
        arrow.append(QPointF(bounds.center().x(), bounds.bottom()));
        arrow.append(bounds.bottomLeft());
        arrow.append(bounds.topLeft());

        p.addPolygon(arrow);
        break;
      }
        NS_DEFAULT_CASE_NOT_IMPLEMENTED;
    }

    setPath(p);
  }

  UpdatePinColors();
}

QPointF nsQtPin::GetPinPos() const
{
  return mapToScene(m_PinCenter);
}

QPointF nsQtPin::GetPinDir() const
{
  if (m_pPin->GetType() == nsPin::Type::Input)
  {
    return QPointF(-1.0f, 0.0f);
  }
  else
  {
    return QPointF(1.0f, 0.0f);
    ;
  }
}

QRectF nsQtPin::GetPinRect() const
{
  auto rectLabel = m_pLabel->boundingRect();
  rectLabel.translate(m_pLabel->pos());

  if (m_pPin->GetType() == nsPin::Type::Input)
  {
    rectLabel.adjust(-5, 0, 0, 0);
  }
  else
  {
    rectLabel.adjust(0, 0, 5, 0);
  }
  return rectLabel;
}

void nsQtPin::UpdateConnections()
{
  for (nsQtConnection* pConnection : m_Connections)
  {
    if (m_pPin->GetType() == nsPin::Type::Input)
    {
      pConnection->SetDirIn(GetPinDir());
      pConnection->SetPosIn(GetPinPos());
    }
    else
    {
      pConnection->SetDirOut(GetPinDir());
      pConnection->SetPosOut(GetPinPos());
    }
  }
}

void nsQtPin::SetHighlightState(nsQtPinHighlightState state)
{
  if (m_HighlightState != state)
  {
    m_HighlightState = state;

    if (UpdatePinColors())
    {
      update();
    }
  }
}

void nsQtPin::SetActive(bool bActive)
{
  m_bIsActive = bActive;

  if (UpdatePinColors())
  {
    update();
  }
}

bool nsQtPin::UpdatePinColors(const nsColorGammaUB* pOverwriteColor)
{
  nsColorGammaUB pinColor = pOverwriteColor != nullptr ? *pOverwriteColor : GetPin()->GetColor();
  QColor base = QApplication::palette().window().color();

  if (!m_bIsActive)
    pinColor = nsMath::Lerp<nsColor>(nsColorGammaUB(base.red(), base.green(), base.blue()), pinColor, 0.2f);

  switch (m_HighlightState)
  {
    case nsQtPinHighlightState::None:
    {
      QPen p = pen();
      p.setColor(nsToQtColor(pinColor));
      setPen(p);

      setBrush(HasAnyConnections() ? pen().color().darker(125) : base);
    }
    break;

    case nsQtPinHighlightState::CannotConnect:
    case nsQtPinHighlightState::CannotConnectSameDirection:
    {
      QPen p = pen();
      p.setColor(base.lighter());
      setPen(p);

      setBrush(base);
    }
    break;

    case nsQtPinHighlightState::CanReplaceConnection:
    case nsQtPinHighlightState::CanAddConnection:
    {
      QPen p = pen();
      p.setColor(nsToQtColor(pinColor));
      setPen(p);

      setBrush(base);
    }
    break;
  }

  QColor labelColor = QApplication::palette().buttonText().color();
  if (!m_bIsActive)
    labelColor = labelColor.darker(150);

  m_pLabel->setDefaultTextColor(labelColor);

  return true;
}

QVariant nsQtPin::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (change == QGraphicsItem::ItemScenePositionHasChanged)
  {
    UpdateConnections();
  }

  return QGraphicsPathItem::itemChange(change, value);
}
