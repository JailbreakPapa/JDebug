#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/ui_ImageWidget.h>
#include <QGraphicsScene>

class QGraphicsPixmapItem;

class WD_GUIFOUNDATION_DLL wdQtImageScene : public QGraphicsScene
{
public:
  wdQtImageScene(QObject* pParent = nullptr);

  void SetImage(QPixmap pixmap);

private:
  QPixmap m_Pixmap;
  QGraphicsPixmapItem* m_pImageItem;
};

class WD_GUIFOUNDATION_DLL wdQtImageWidget : public QWidget, public Ui_ImageWidget
{
  Q_OBJECT

public:
  wdQtImageWidget(QWidget* pParent, bool bShowButtons = true);
  ~wdQtImageWidget();

  void SetImage(QPixmap pixmap);

  void SetImageSize(float fScale = 1.0f);
  void ScaleImage(float fFactor);

private Q_SLOTS:

  void on_ButtonZoomIn_clicked();
  void on_ButtonZoomOut_clicked();
  void on_ButtonResetZoom_clicked();

private:
  void ImageApplyScale();

  wdQtImageScene* m_pScene;
  float m_fCurrentScale;
};

