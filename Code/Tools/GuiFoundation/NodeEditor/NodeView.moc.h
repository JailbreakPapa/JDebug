#pragma once

#include <Foundation/Math/Vec2.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QGraphicsView>

class wdQtNodeScene;

class WD_GUIFOUNDATION_DLL wdQtNodeView : public QGraphicsView
{
  Q_OBJECT
public:
  explicit wdQtNodeView(QWidget* pParent = nullptr);
  ~wdQtNodeView();

  void SetScene(wdQtNodeScene* pScene);
  wdQtNodeScene* GetScene();

protected:
  virtual void mousePressEvent(QMouseEvent* event) override;
  virtual void mouseMoveEvent(QMouseEvent* event) override;
  virtual void mouseReleaseEvent(QMouseEvent* event) override;
  virtual void wheelEvent(QWheelEvent* event) override;
  virtual void contextMenuEvent(QContextMenuEvent* event) override;
  virtual void resizeEvent(QResizeEvent*) override;

private:
  void UpdateView();

private:
  wdQtNodeScene* m_pScene;
  bool m_bPanning;
  wdInt32 m_iPanCounter;

  QPointF m_ViewPos;
  QPointF m_ViewScale;

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
  QPointF m_StartDragView;
#else
  QPoint m_vStartDragView;
#endif

  QPointF m_StartDragScene;
};
