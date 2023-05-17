#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/NodeEditor/Connection.h>
#include <GuiFoundation/NodeEditor/NodeScene.moc.h>
#include <QApplication>
#include <QPalette>

wdQtConnection::wdQtConnection(QGraphicsItem* pParent)
  : QGraphicsPathItem(pParent)
{
  auto palette = QApplication::palette();

  QPen pen(palette.highlightedText().color(), 3, Qt::SolidLine);
  setPen(pen);
  setBrush(Qt::NoBrush);

  m_InDir = QPointF(-1.0f, 0.0f);
  m_OutDir = QPointF(1.0f, 0.0f);
  setZValue(-1);
}

wdQtConnection::~wdQtConnection() = default;

void wdQtConnection::InitConnection(const wdDocumentObject* pObject, const wdConnection* pConnection)
{
  m_pObject = pObject;
  m_pConnection = pConnection;
}

void wdQtConnection::SetPosIn(const QPointF& point)
{
  m_InPoint = point;
  UpdateGeometry();
}

void wdQtConnection::SetPosOut(const QPointF& point)
{
  m_OutPoint = point;
  UpdateGeometry();
}

void wdQtConnection::SetDirIn(const QPointF& dir)
{
  m_InDir = dir;
  UpdateGeometry();
}

void wdQtConnection::SetDirOut(const QPointF& dir)
{
  m_OutDir = dir;
  UpdateGeometry();
}

void wdQtConnection::UpdateGeometry()
{
  constexpr float arrowHalfSize = 8.0f;

  prepareGeometryChange();

  QPainterPath p;
  QPointF dir = m_InPoint - m_OutPoint;

  auto pScene = static_cast<wdQtNodeScene*>(scene());
  if (pScene->GetConnectionStyle() == wdQtNodeScene::ConnectionStyle::StraightLine)
  {
    QPointF startPoint = m_OutPoint;
    QPointF endPoint = m_InPoint;

    if (pScene->GetConnectionDecorationFlags().IsSet(wdQtNodeScene::ConnectionDecorationFlags::DirectionArrows))
    {
      const float length = wdMath::Sqrt(dir.x() * dir.x() + dir.y() * dir.y());
      const float invLength = length != 0.0f ? 1.0f / length : 1.0f;
      const QPointF dirNorm = dir * invLength;
      const QPointF normal = QPointF(dirNorm.y(), -dirNorm.x());

      // offset start and endpoint
      startPoint -= normal * (arrowHalfSize * 1.3f);
      endPoint -= normal * (arrowHalfSize * 1.3f);

      const QPointF midPoint = startPoint + dir * 0.5f;
      const QPointF tipPoint = midPoint + dirNorm * arrowHalfSize;
      const QPointF backPoint = midPoint - dirNorm * arrowHalfSize;

      QPolygonF arrow;
      arrow.append(tipPoint);
      arrow.append(backPoint + normal * arrowHalfSize);
      arrow.append(backPoint - normal * arrowHalfSize);
      arrow.append(tipPoint);

      p.addPolygon(arrow);
    }

    p.moveTo(startPoint);
    p.lineTo(endPoint);
  }
  else
  {
    p.moveTo(m_OutPoint);
    float fDotOut = QPointF::dotProduct(m_OutDir, dir);
    float fDotIn = QPointF::dotProduct(m_InDir, -dir);

    fDotOut = wdMath::Max(100.0f, wdMath::Abs(fDotOut));
    fDotIn = wdMath::Max(100.0f, wdMath::Abs(fDotIn));

    QPointF ctr1 = m_OutPoint + m_OutDir * (fDotOut * 0.5f);
    QPointF ctr2 = m_InPoint + m_InDir * (fDotIn * 0.5f);

    p.cubicTo(ctr1, ctr2, m_InPoint);
  }

  setPath(p);
}

QPen wdQtConnection::DeterminePen() const
{
  if (m_pConnection == nullptr)
  {
    return pen();
  }

  wdColorGammaUB color;
  const wdColorGammaUB sourceColor = m_pConnection->GetSourcePin().GetColor();
  const wdColorGammaUB targetColor = m_pConnection->GetTargetPin().GetColor();

  const bool isSourceGrey = (sourceColor.r == sourceColor.g && sourceColor.r == sourceColor.b);
  const bool isTargetGrey = (targetColor.r == targetColor.g && targetColor.r == targetColor.b);

  if (!isSourceGrey)
  {
    color = wdMath::Lerp(sourceColor, targetColor, 0.2f);
  }
  else if (!isTargetGrey)
  {
    color = wdMath::Lerp(sourceColor, targetColor, 0.8f);
  }
  else
  {
    color = wdMath::Lerp(sourceColor, targetColor, 0.5f);
  }

  if (m_bAdjacentNodeSelected)
  {
    color = wdMath::Lerp(color, wdColorGammaUB(255, 255, 255), 0.1f);
    return QPen(QBrush(wdToQtColor(color)), 3, Qt::DashLine);
  }
  else
  {
    return QPen(QBrush(wdToQtColor(color)), 2, Qt::SolidLine);
  }
}

void wdQtConnection::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  auto palette = QApplication::palette();

  QPen p = DeterminePen();
  if (isSelected())
  {
    p.setColor(palette.highlight().color());
  }
  painter->setPen(p);

  auto decorationFlags = static_cast<wdQtNodeScene*>(scene())->GetConnectionDecorationFlags();
  if (decorationFlags.IsSet(wdQtNodeScene::ConnectionDecorationFlags::DirectionArrows))
  {
    painter->setBrush(p.brush());
  }

  painter->drawPath(path());
}
