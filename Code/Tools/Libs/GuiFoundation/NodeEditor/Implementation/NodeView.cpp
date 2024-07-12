/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/NodeEditor/NodeScene.moc.h>
#include <GuiFoundation/NodeEditor/NodeView.moc.h>
#include <QMouseEvent>

nsQtNodeView::nsQtNodeView(QWidget* pParent)
  : QGraphicsView(pParent)

{
  setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
  setDragMode(QGraphicsView::DragMode::RubberBandDrag);

  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_ViewPos = QPointF(0, 0);
  m_ViewScale = QPointF(1, 1);
}

nsQtNodeView::~nsQtNodeView() = default;

void nsQtNodeView::SetScene(nsQtNodeScene* pScene)
{
  m_pScene = pScene;
  setScene(pScene);

  QRectF sceneRect = m_pScene->sceneRect();
  m_ViewPos = sceneRect.topLeft();
  m_ViewScale = QPointF(1, 1);
  UpdateView();
}

nsQtNodeScene* nsQtNodeView::GetScene()
{
  return m_pScene;
}

void nsQtNodeView::mousePressEvent(QMouseEvent* event)
{
  QGraphicsView::mousePressEvent(event);

  if (event->button() == Qt::RightButton)
  {
    setContextMenuPolicy(Qt::NoContextMenu);
    m_StartDragView = event->position();

    m_StartDragScene = m_ViewPos;
    viewport()->setCursor(Qt::ClosedHandCursor);
    event->accept();
    m_bPanning = true;
    m_iPanCounter = 0;
  }
}

void nsQtNodeView::mouseMoveEvent(QMouseEvent* event)
{
  QGraphicsView::mouseMoveEvent(event);

  if (m_bPanning)
  {
    m_iPanCounter++;
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    QPointF vViewDelta = m_StartDragView - event->position();
#else
    QPointF vViewDelta = m_vStartDragView - event->pos();
#endif
    QPointF vSceneDelta = QPointF(vViewDelta.x() / m_ViewScale.x(), vViewDelta.y() / m_ViewScale.y());
    m_ViewPos = m_StartDragScene + vSceneDelta;
    UpdateView();
  }
}

void nsQtNodeView::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() == Qt::RightButton && m_bPanning)
  {
    viewport()->setCursor(Qt::ArrowCursor);
    event->accept();
    m_bPanning = false;
    setContextMenuPolicy(Qt::DefaultContextMenu);
    if (m_iPanCounter > 2)
      return;
  }
  QGraphicsView::mouseReleaseEvent(event);
}

void nsQtNodeView::wheelEvent(QWheelEvent* event)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
  QPointF centerA(event->position().x() / m_ViewScale.x(), event->position().y() / m_ViewScale.y());
#else
  QPointF centerA(event->pos().x() / m_ViewScale.x(), event->pos().y() / m_ViewScale.y());
#endif

  const qreal fScaleFactor = 1.15;
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
  const qreal fScale = (event->angleDelta().y() > 0) ? fScaleFactor : (1.0 / fScaleFactor);
#else
  const qreal fScale = (event->delta() > 0) ? fScaleFactor : (1.0 / fScaleFactor);
#endif

  m_ViewScale *= fScale;
  m_ViewScale.setX(nsMath::Clamp(m_ViewScale.x(), 0.01, 2.0));
  m_ViewScale.setY(nsMath::Clamp(m_ViewScale.y(), 0.01, 2.0));

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
  QPointF centerB(event->position().x() / m_ViewScale.x(), event->position().y() / m_ViewScale.y());
#else
  QPointF centerB(event->pos().x() / m_ViewScale.x(), event->pos().y() / m_ViewScale.y());
#endif
  m_ViewPos -= (centerB - centerA);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
  m_StartDragView = event->position();
#else
  m_vStartDragView = event->pos();
#endif
  m_StartDragScene = m_ViewPos;

  UpdateView();
}

void nsQtNodeView::contextMenuEvent(QContextMenuEvent* event)
{
  if (m_iPanCounter > 2)
  {
    m_iPanCounter = 0;
    return;
  }
  QGraphicsView::contextMenuEvent(event);
}

void nsQtNodeView::resizeEvent(QResizeEvent* event)
{
  QGraphicsView::resizeEvent(event);
  UpdateView();
}

void nsQtNodeView::UpdateView()
{
  QRectF sceneRect(m_ViewPos.x(), m_ViewPos.y(), width() / m_ViewScale.x(), height() / m_ViewScale.y());
  setSceneRect(sceneRect);
  fitInView(sceneRect, Qt::KeepAspectRatio);
}
