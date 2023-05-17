#pragma once

#include <Foundation/Types/Delegate.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QWidget>

class QPaintEvent;

class WD_GUIFOUNDATION_DLL wdQGridBarWidget : public QWidget
{
  Q_OBJECT

public:
  wdQGridBarWidget(QWidget* pParent);

  void SetConfig(const QRectF& viewportSceneRect, double fTextGridStops, double fFineGridStops, wdDelegate<QPointF(const QPointF&)> mapFromSceneFunc);

protected:
  virtual void paintEvent(QPaintEvent* event) override;

private:
  QRectF m_ViewportSceneRect;
  double m_fTextGridStops;
  double m_fFineGridStops;
  wdDelegate<QPointF(const QPointF&)> m_MapFromSceneFunc;
};

