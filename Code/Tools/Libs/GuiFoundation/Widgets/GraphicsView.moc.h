/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <Foundation/Types/Delegate.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QGraphicsView>

class QWheelEvent;
class QMouseEvent;
class QKeyEvent;
class nsQGridBarWidget;

class NS_GUIFOUNDATION_DLL nsQtGraphicsView : public QGraphicsView
{
  Q_OBJECT

public:
  nsQtGraphicsView(QWidget* pParent = nullptr);

  virtual void wheelEvent(QWheelEvent* e) override;
  virtual void mousePressEvent(QMouseEvent* e) override;
  virtual void mouseReleaseEvent(QMouseEvent* e) override;
  virtual void mouseMoveEvent(QMouseEvent* e) override;
  virtual void keyPressEvent(QKeyEvent* e) override;

  void SetZoom(float fZoom);
  float GetZoom() const { return m_fZoom; }

  void SetZoomLimits(float fMinZoom, float fMaxZoom);

Q_SIGNALS:
  void BeginDrag();
  void EndDrag();
  void DeleteCPs();

protected:
  void UpdateTransform();

  bool m_bPanning;
  bool m_bForwardMouseEvents;
  bool m_bDragging;

  float m_fMinZoom, m_fMaxZoom;
  float m_fZoom;
  QPoint m_LastGlobalMouseMovePos;
};

