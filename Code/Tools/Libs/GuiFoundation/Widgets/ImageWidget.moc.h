/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/ui_ImageWidget.h>
#include <QGraphicsScene>

class QGraphicsPixmapItem;

class NS_GUIFOUNDATION_DLL nsQtImageScene : public QGraphicsScene
{
public:
  nsQtImageScene(QObject* pParent = nullptr);

  void SetImage(QPixmap pixmap);

private:
  QPixmap m_Pixmap;
  QGraphicsPixmapItem* m_pImageItem;
};

class NS_GUIFOUNDATION_DLL nsQtImageWidget : public QWidget, public Ui_ImageWidget
{
  Q_OBJECT

public:
  nsQtImageWidget(QWidget* pParent, bool bShowButtons = true);
  ~nsQtImageWidget();

  void SetImage(QPixmap pixmap);

  void SetImageSize(float fScale = 1.0f);
  void ScaleImage(float fFactor);

private Q_SLOTS:

  void on_ButtonZoomIn_clicked();
  void on_ButtonZoomOut_clicked();
  void on_ButtonResetZoom_clicked();

private:
  void ImageApplyScale();

  nsQtImageScene* m_pScene;
  float m_fCurrentScale;
};

