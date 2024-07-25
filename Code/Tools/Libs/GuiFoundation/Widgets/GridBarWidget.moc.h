#pragma once

#include <Foundation/Types/Delegate.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QWidget>

class QPaintEvent;

class NS_GUIFOUNDATION_DLL nsQGridBarWidget : public QWidget
{
  Q_OBJECT

public:
  nsQGridBarWidget(QWidget* pParent);

  void SetConfig(const QRectF& viewportSceneRect, double fTextGridStops, double fFineGridStops, nsDelegate<QPointF(const QPointF&)> mapFromSceneFunc);

protected:
  virtual void paintEvent(QPaintEvent* event) override;

private:
  QRectF m_ViewportSceneRect;
  double m_fTextGridStops;
  double m_fFineGridStops;
  nsDelegate<QPointF(const QPointF&)> m_MapFromSceneFunc;
};
